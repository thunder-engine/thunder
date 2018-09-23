#include "engine.h"

#include <string>
#include <sstream>

#include <log.h>
#include <file.h>

#include <objectsystem.h>
#include <bson.h>
#include <json.h>
#include <metatype.h>
#include <uri.h>

#include "module.h"
#include "system.h"
#include "controller.h"
#include "timer.h"
#include "input.h"

#include "components/scene.h"
#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"
#include "components/staticmesh.h"
#include "components/directlight.h"
#include "components/textmesh.h"

#include "components/spritemesh.h"

#include "components/animationcontroller.h"

#include "analytics/profiler.h"
#if THUNDER_MOBILE
    #include "adapters/mobileadaptor.h"
#else
    #include "adapters/desktopadaptor.h"
#endif
#include "resources/text.h"
#include "resources/texture.h"
#include "resources/rendertexture.h"
#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/font.h"
#include "resources/animationclip.h"
#include "resources/pipeline.h"

#include "log.h"

const char *gIndex("index");

const char *gContent("content");
const char *gSettings("settings");

const char *gEntry(".entry");

class EnginePrivate {
public:
    static unordered_map<string, string>    m_IndexMap;
    static unordered_map<string, Object*>   m_ResourceCache;
    static unordered_map<Object*, string>   m_ReferenceCache;

    EnginePrivate() :
        m_pFile(nullptr),
        m_Controller(nullptr),
        m_Valid(false) {

    }

    list<ISystem *>             m_Systems;

    IController                *m_Controller;

    IFile                      *m_pFile;

    bool                        m_Valid;

    string                      m_EntryLevel;

    static string               m_ApplicationPath;

    static string               m_ApplicationDir;

    static string               m_Organization;

    static string               m_Application;

    static IPlatformAdaptor    *m_pPlatform;

    static VariantMap           m_Values;
};

unordered_map<string, string>   EnginePrivate::m_IndexMap;
unordered_map<string, Object*>  EnginePrivate::m_ResourceCache;
unordered_map<Object*, string>  EnginePrivate::m_ReferenceCache;
VariantMap                      EnginePrivate::m_Values;
string                          EnginePrivate::m_ApplicationPath;
string                          EnginePrivate::m_ApplicationDir;
string                          EnginePrivate::m_Organization;
string                          EnginePrivate::m_Application;
IPlatformAdaptor               *EnginePrivate::m_pPlatform;

Engine::Engine(IFile *file, int, char **argv) :
        p_ptr(new EnginePrivate()) {
    PROFILER_MARKER;

    EnginePrivate::m_ApplicationPath    = argv[0];
    Uri uri(EnginePrivate::m_ApplicationPath);
    EnginePrivate::m_ApplicationDir     = uri.dir();
    EnginePrivate::m_Application        = uri.baseName();

    p_ptr->m_pFile  = file;

#if THUNDER_MOBILE
    p_ptr->m_pPlatform  = new MobileAdaptor(this);
#else
    p_ptr->m_pPlatform  = new DesktopAdaptor(this);
#endif

    p_ptr->m_Controller = new IController();

    Text::registerClassFactory();
    Texture::registerClassFactory();
    Material::registerClassFactory();
    Mesh::registerClassFactory();
    Atlas::registerClassFactory();
    Font::registerClassFactory();
    AnimationClip::registerClassFactory();

    Scene::registerClassFactory();
    Actor::registerClassFactory();
    Transform::registerClassFactory();
    Camera::registerClassFactory();

    StaticMesh::registerClassFactory();
    TextMesh::registerClassFactory();
    SpriteMesh::registerClassFactory();
    DirectLight::registerClassFactory();
    RenderTexture::registerClassFactory();

    AnimationController::registerClassFactory();

    Pipeline::registerClassFactory();

    registerMetaType<MaterialArray>("MaterialArray");
}

Engine::~Engine() {
    PROFILER_MARKER;

    Input::destroy();
    p_ptr->m_pPlatform->destroy();
}

/*!
    Initialize all engine systems.
*/
bool Engine::init() {
    PROFILER_MARKER;

    bool result     = p_ptr->m_pPlatform->init();

    reloadBundle();

    Timer::init(1.0 / 60.0);
    Input::instance()->init(p_ptr->m_pPlatform);

    return result;
}

int32_t Engine::exec() {
    PROFILER_MARKER;

    for(auto it : p_ptr->m_Systems) {
        if(!it->init()) {
            Log(Log::ERR) << "Failed to initialize system:" << it->name();
            p_ptr->m_pPlatform->stop();
            return false;
        }
    }

    Scene *scene   = Engine::objectCreate<Scene>();
    if(scene) {
        p_ptr->m_Valid  = true;

        string path     = value(gEntry, "").toString();
        Actor *level    = loadResource<Actor>(path);
        Log(Log::DBG) << "Level:" << path.c_str() << "loading...";
        if(level) {
            level->setParent(scene);
        }

        Log(Log::DBG) << "Looking camera...";
        Camera *component   = scene->findChild<Camera *>();
        if(component == nullptr) {
            Log(Log::DBG) << "Camera not found creating new one.";
            Actor *camera   = Engine::createActor("ActiveCamera", scene);
            camera->transform()->setPosition(Vector3(0.0f));
            component       = camera->addComponent<Camera>();
        }
        component->pipeline()->resize(p_ptr->m_pPlatform->screenWidth(), p_ptr->m_pPlatform->screenHeight());
        component->setRatio(float(p_ptr->m_pPlatform->screenWidth()) / float(p_ptr->m_pPlatform->screenHeight()));

        p_ptr->m_Controller->setActiveCamera(component);
        // Enter to game loop
        while(p_ptr->m_Valid) {
            Timer::update();
            double lag  = Timer::deltaTime();
            while(lag >= Timer::fixedDelta()) {
                // fixed update
                p_ptr->m_Controller->update();
                updateScene(scene);

                lag -= Timer::fixedDelta();
            }
            for(auto it : p_ptr->m_Systems) {
                it->update(*scene);
            }
            p_ptr->m_Valid  = p_ptr->m_pPlatform->isValid();
            p_ptr->m_pPlatform->update();
        }
        p_ptr->m_pPlatform->stop();
        delete scene;
    }
    return 0;
}

Variant Engine::value(const string &key, const Variant &defaultValue) {
    PROFILER_MARKER;

    auto it = EnginePrivate::m_Values.find(key);
    if(it != EnginePrivate::m_Values.end()) {
        return it->second;
    }
    return defaultValue;
}

void Engine::setValue(const string &key, const Variant &value) {
    PROFILER_MARKER;

    EnginePrivate::m_Values[key]    = value;
}

Object *Engine::loadResource(const string &path) {
    PROFILER_MARKER;

    if(!path.empty()) {
        string uuid = path;
        {
            auto it = EnginePrivate::m_IndexMap.find(path);
            if(it != EnginePrivate::m_IndexMap.end()) {
                uuid    = it->second;
            }
        }
        {
            auto it = EnginePrivate::m_ResourceCache.find(uuid);
            if(it != EnginePrivate::m_ResourceCache.end() && it->second) {
                return it->second;
            } else {
                IFile *file = ((Engine *)Engine::instance())->file();
                _FILE *fp   = file->_fopen(uuid.c_str(), "r");
                if(fp) {
                    ByteArray data;
                    data.resize(file->_fsize(fp));
                    file->_fread(&data[0], data.size(), 1, fp);
                    file->_fclose(fp);

                    Variant var     = Bson::load(data);
                    if(!var.isValid()) {
                        var = Json::load(string(data.begin(), data.end()));
                    }
                    if(var.isValid()) {
                        Object *res = Engine::toObject(var);
                        if(res) {
                            EnginePrivate::m_ResourceCache[uuid]    = res;
                            EnginePrivate::m_ReferenceCache[res]    = uuid;
                            return res;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

string Engine::reference(Object *object) {
    PROFILER_MARKER;

    auto it = EnginePrivate::m_ReferenceCache.find(object);
    if(it != EnginePrivate::m_ReferenceCache.end()) {
        return it->second;
    }
    return string();
}

void Engine::reloadBundle() {
    PROFILER_MARKER;
    EnginePrivate::m_IndexMap.clear();

    IFile *file = ((Engine *)Engine::instance())->file();
    _FILE *fp   = file->_fopen(gIndex, "r");
    if(fp) {
        ByteArray data;
        data.resize(file->_fsize(fp));
        file->_fread(&data[0], data.size(), 1, fp);
        file->_fclose(fp);

        Variant var = Json::load(string(data.begin(), data.end()));
        if(var.isValid()) {
            VariantMap root    = var.toMap();

            for(auto it : root[gContent].toMap()) {
                EnginePrivate::m_IndexMap[it.second.toString()] = it.first;
            }

            for(auto it : root[gSettings].toMap()) {
                EnginePrivate::m_Values[it.first]   = it.second;
            }
        }
    }
}

Actor *Engine::createActor(const string &name, Object *parent, const StringList &components) {
    Actor *result   = Engine::objectCreate<Actor>(name, parent);
    Engine::objectCreate<Transform>("", result);
    for(auto &it : components) {
        Engine::objectCreate(it, "", result);
    }

    return result;
}

void Engine::addModule(IModule *mode) {
    PROFILER_MARKER;
    if(mode->types() & IModule::SYSTEM) {
        p_ptr->m_Systems.push_back(mode->system());
    }
}

bool Engine::createWindow() {
    PROFILER_MARKER;
    return p_ptr->m_pPlatform->start();
}

IController *Engine::controller() {
    PROFILER_MARKER;

    return p_ptr->m_Controller;
}

IFile *Engine::file() {
    PROFILER_MARKER;

    return p_ptr->m_pFile;
}

string Engine::locationAppDir() {
    return EnginePrivate::m_ApplicationDir;
}

string Engine::locationConfig() {
    return EnginePrivate::m_pPlatform->locationLocalDir();
}

string Engine::locationAppConfig() {
    string result;
    if(!EnginePrivate::m_Organization.empty()) {
        result  += "/" + EnginePrivate::m_Organization;
    }
    if(!EnginePrivate::m_Application.empty()) {
        result  += "/" + EnginePrivate::m_Application;
    }
    return result;
}

string Engine::applicationName() const {
    return EnginePrivate::m_Application;
}

void Engine::setApplicationName(const string &name) {
    EnginePrivate::m_Application    = name;
}

string Engine::organizationName() const {
    return EnginePrivate::m_Organization;
}

void Engine::setOrganizationName(const string &name) {
    EnginePrivate::m_Organization   = name;
}

void Engine::updateScene(Object *object) {
    if(object) {
        for(auto &it : object->getChildren()) {
            Object *child   = it;
            Component *comp = dynamic_cast<Component *>(child);
            if(comp && comp->isEnable()) {
                if(!comp->isStarted()) {
                    comp->start();
                    comp->setStarted(true);
                }
                comp->update();
            } else {
                updateScene(child);
            }
        }
    }
}
