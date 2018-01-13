#include "engine.h"

#if __APPLE__
    #include <TargetConditionals.h>
#elif __ANDROID__
    #include <android/api-level.h>
#endif

#include <string>
#include <sstream>

#include <log.h>
#include <file.h>

#include <aobjectsystem.h>
#include <abson.h>
#include <ajson.h>

#include "module.h"
#include "system.h"
#include "controller.h"
#include "timer.h"
#include "input.h"

#include "components/chunk.h"
#include "components/scene.h"
#include "components/actor.h"
#include "components/camera.h"
#include "components/staticmesh.h"
#include "components/lightsource.h"

#include "components/sprite.h"

#include "analytics/profiler.h"

#include "adapters/desktopadaptor.h"

#include "resources/text.h"
#include "resources/texture.h"
#include "resources/material.h"
#include "resources/mesh.h"

#include "log.h"
#include "ametatype.h"

const char *gIndex(".index");

const char *gContent("content");
const char *gSettings("settings");

const char *gEntry(".entry");


class EnginePrivate {
public:
    static unordered_map<string, string>    m_IndexMap;
    static unordered_map<string, AObject*>  m_ResourceCache;
    static unordered_map<AObject*, string>  m_ReferenceCache;

    EnginePrivate() :
        m_pFile(nullptr),
        m_Controller(nullptr),
        m_pPlatform(nullptr),
        m_Valid(false) {

    }

    list<ISystem *>             m_Systems;

    IController                *m_Controller;

    IFile                      *m_pFile;

    IPlatformAdaptor           *m_pPlatform;

    static AVariantMap          m_Values;

    bool                        m_Valid;

    string                      m_EntryLevel;

};

unordered_map<string, string>   EnginePrivate::m_IndexMap;
unordered_map<string, AObject*> EnginePrivate::m_ResourceCache;
unordered_map<AObject*, string> EnginePrivate::m_ReferenceCache;
AVariantMap                     EnginePrivate::m_Values;

Engine::Engine(IFile *file) :
        p_ptr(new EnginePrivate()) {
    PROFILER_MARKER

    p_ptr->m_pFile  = file;

#if TARGET_OS_IPHONE

#elif __ANDROID__

#else
    p_ptr->m_pPlatform  = new DesktopAdaptor();
#endif

    p_ptr->m_Controller = new IController(this);

    Text::registerClassFactory();
    Texture::registerClassFactory();
    Material::registerClassFactory();
    Mesh::registerClassFactory();

    Scene::registerClassFactory();
    Chunk::registerClassFactory();
    Actor::registerClassFactory();
    Camera::registerClassFactory();

    StaticMesh::registerClassFactory();
    Sprite::registerClassFactory();
    LightSource::registerClassFactory();

    registerMetaType<MaterialArray>("MaterialArray");
}

Engine::~Engine() {
    PROFILER_MARKER

    Input::destroy();
    p_ptr->m_pPlatform->destroy();
}

/*!
    Initialize all engine systems.
*/
bool Engine::init() {
    PROFILER_MARKER

    bool result     = p_ptr->m_pPlatform->init(this);

    reloadBundle();

    Timer::init(1.0 / 60.0);
    Input::instance()->init(p_ptr->m_pPlatform);

    return result;
}

int32_t Engine::exec() {
    PROFILER_MARKER

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

        Chunk *level    = loadResource<Chunk>(value(gEntry, "").toString());
        if(level) {
            level->setParent(scene);
        }

        Actor *camera   = Engine::objectCreate<Actor>("ActiveCamera", scene);
        camera->setPosition(Vector3(0.0f, 0.0f, 20.0f));
        Camera *component   = camera->addComponent<Camera>();
        component->resize(p_ptr->m_pPlatform->screenWidth(), p_ptr->m_pPlatform->screenHeight());
        p_ptr->m_Controller->setActiveCamera(component);

        // Enter to game loop
        while(p_ptr->m_Valid) {
            Timer::update();
            double lag  = Timer::deltaTime();
            while(lag >= Timer::fixedDelta()) {
                // fixed update
                p_ptr->m_Controller->update();
                scene->update();

                for(auto it : p_ptr->m_Systems) {
                    it->update(*scene);
                }
                lag -= Timer::fixedDelta();
            }
            p_ptr->m_Valid  = p_ptr->m_pPlatform->isValid();
            p_ptr->m_pPlatform->update();
        }
        p_ptr->m_pPlatform->stop();
        delete scene;
    }
    return 0;
}

AVariant Engine::value(const string &key, const AVariant &defaultValue) {
    PROFILER_MARKER

    auto it = EnginePrivate::m_Values.find(key);
    if(it != EnginePrivate::m_Values.end()) {
        return it->second;
    }
    return defaultValue;
}

void Engine::setValue(const string &key, const AVariant &value) {
    PROFILER_MARKER

    EnginePrivate::m_Values[key]    = value;
}

AObject *Engine::loadResource(const string &path) {
    PROFILER_MARKER

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
                    AByteArray data;
                    data.resize(file->_fsize(fp));
                    file->_fread(&data[0], data.size(), 1, fp);
                    file->_fclose(fp);

                    uint32_t offset = 0;
                    AVariant var    = ABson::load(data, offset);
                    if(!var.isValid()) {
                        var = AJson::load(string(data.begin(), data.end()));
                    }
                    if(var.isValid()) {
                        AObject *res    = Engine::toObject(var);
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

string Engine::reference(AObject *object) {
    PROFILER_MARKER

    auto it = EnginePrivate::m_ReferenceCache.find(object);
    if(it != EnginePrivate::m_ReferenceCache.end()) {
        return it->second;
    }
    return string();
}

void Engine::reloadBundle() {
    PROFILER_MARKER
    EnginePrivate::m_IndexMap.clear();

    IFile *file = ((Engine *)Engine::instance())->file();
    _FILE *fp   = file->_fopen(gIndex, "r");
    if(fp) {
        AByteArray data;
        data.resize(file->_fsize(fp));
        file->_fread(&data[0], data.size(), 1, fp);
        file->_fclose(fp);

        AVariant var = AJson::load(string(data.begin(), data.end()));
        if(var.isValid()) {
            AVariantMap root    = var.toMap();

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
    PROFILER_MARKER
    if(mode->types() && IModule::SYSTEM) {
        p_ptr->m_Systems.push_back(mode->system());
    }
}

bool Engine::createWindow() {
    PROFILER_MARKER
    return p_ptr->m_pPlatform->start();
}

IController *Engine::controller() {
    PROFILER_MARKER

    return p_ptr->m_Controller;
}

IFile *Engine::file() {
    PROFILER_MARKER

    return p_ptr->m_pFile;
}
