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
#include "timer.h"
#include "input.h"

#include "components/scene.h"
#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "components/staticmesh.h"
#include "components/textmesh.h"
#include "components/spritemesh.h"
#include "components/particlerender.h"
#include "components/directlight.h"
#include "components/pointlight.h"

#include "components/animationcontroller.h"

#include "analytics/profiler.h"
#ifdef THUNDER_MOBILE
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

#include "resources/particleeffect.h"

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
        m_Valid(false) {

    }

    Scene                       *m_pScene;

    list<ISystem *>             m_Systems;

    static IFile               *m_pFile;

    bool                        m_Valid;

    string                      m_EntryLevel;

    static string               m_ApplicationPath;

    static string               m_ApplicationDir;

    static string               m_Organization;

    static string               m_Application;

    static IPlatformAdaptor    *m_pPlatform;

    static VariantMap           m_Values;
};


IFile                          *EnginePrivate::m_pFile  = nullptr;

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

#ifdef THUNDER_MOBILE
    p_ptr->m_pPlatform  = new MobileAdaptor(this);
#else
    p_ptr->m_pPlatform  = new DesktopAdaptor(this);
#endif

    ObjectSystem system;

    Text::registerClassFactory(&system);
    Texture::registerClassFactory(&system);
    Material::registerClassFactory(&system);
    Mesh::registerClassFactory(&system);
    Atlas::registerClassFactory(&system);
    Font::registerClassFactory(&system);
    AnimationClip::registerClassFactory(&system);

    Scene::registerClassFactory(&system);
    Actor::registerClassFactory(&system);
    Transform::registerClassFactory(&system);
    Camera::registerClassFactory(&system);

    StaticMesh::registerClassFactory(&system);
    TextMesh::registerClassFactory(&system);
    SpriteMesh::registerClassFactory(&system);
    DirectLight::registerClassFactory(&system);
    PointLight::registerClassFactory(&system);
    RenderTexture::registerClassFactory(&system);

    ParticleRender::registerClassFactory(&system);
    ParticleEffect::registerClassFactory(&system);

    AnimationController::registerClassFactory(&system);

    Pipeline::registerClassFactory(&system);

    p_ptr->m_pScene = Engine::objectCreate<Scene>("Scene");

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

    Timer::init(1.0f / 60.0f);
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


    if(p_ptr->m_pScene) {
        p_ptr->m_Valid  = true;

        string path     = value(gEntry, "").toString();
        Actor *level    = loadResource<Actor>(path);
        Log(Log::DBG) << "Level:" << path.c_str() << "loading...";
        if(level) {
            level->setParent(p_ptr->m_pScene);
        }

        Log(Log::DBG) << "Looking camera...";
        Camera *component   = p_ptr->m_pScene->findChild<Camera *>();
        if(component == nullptr) {
            Log(Log::DBG) << "Camera not found creating new one.";
            Actor *camera   = Engine::objectCreate<Actor>("ActiveCamera", p_ptr->m_pScene);
            camera->transform()->setPosition(Vector3(0.0f));
            component       = camera->addComponent<Camera>();
        }
        component->pipeline()->resize(p_ptr->m_pPlatform->screenWidth(), p_ptr->m_pPlatform->screenHeight());
        component->setRatio(float(p_ptr->m_pPlatform->screenWidth()) / float(p_ptr->m_pPlatform->screenHeight()));

        Camera::setCurrent(component);
        // Enter to game loop
        while(p_ptr->m_Valid) {
            Timer::update();
            float lag  = Timer::deltaTime();
            while(lag >= Timer::fixedDelta()) {
                // fixed update
                updateScene(p_ptr->m_pScene);

                lag -= Timer::fixedDelta();
            }
            for(auto it : p_ptr->m_Systems) {
                it->update(*(p_ptr->m_pScene));
            }
            p_ptr->m_Valid  = p_ptr->m_pPlatform->isValid();
            p_ptr->m_pPlatform->update();
        }
        p_ptr->m_pPlatform->stop();
        delete p_ptr->m_pScene;
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
                IFile *file = Engine::file();
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
                            setResource(res, uuid);
                            return res;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

void Engine::setResource(Object *object, string &uuid) {
    PROFILER_MARKER;

    EnginePrivate::m_ResourceCache[uuid]    = object;
    EnginePrivate::m_ReferenceCache[object] = uuid;
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

    IFile *file = Engine::file();
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

Scene *Engine::scene() {
    PROFILER_MARKER;
    return p_ptr->m_pScene;
}

IFile *Engine::file() {
    PROFILER_MARKER;
    return EnginePrivate::m_pFile;
}

string Engine::locationAppDir() {
    PROFILER_MARKER;

    return EnginePrivate::m_ApplicationDir;
}

string Engine::locationConfig() {
    PROFILER_MARKER;

    return EnginePrivate::m_pPlatform->locationLocalDir();
}

string Engine::locationAppConfig() {
    PROFILER_MARKER;

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
    PROFILER_MARKER;

    return EnginePrivate::m_Application;
}

void Engine::setApplicationName(const string &name) {
    PROFILER_MARKER;

    EnginePrivate::m_Application    = name;
}

string Engine::organizationName() const {
    PROFILER_MARKER;

    return EnginePrivate::m_Organization;
}

void Engine::setOrganizationName(const string &name) {
    PROFILER_MARKER;

    EnginePrivate::m_Organization   = name;
}

void Engine::updateScene(Object *object) {
    PROFILER_MARKER;

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
