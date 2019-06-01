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

#include "components/meshrender.h"
#include "components/textrender.h"
#include "components/spriterender.h"
#include "components/particlerender.h"
#include "components/directlight.h"
#include "components/pointlight.h"
#include "components/spotlight.h"

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
#include "resources/animationstatemachine.h"
#include "resources/pipeline.h"

#include "resources/particleeffect.h"

#include "commandbuffer.h"

#include "log.h"

static const char *gIndex("index");

static const char *gContent("content");
static const char *gSettings("settings");

static const char *gEntry(".entry");
static const char *gCompany(".company");
static const char *gProject(".project");

class EnginePrivate {
public:
    static StringMap                        m_IndexMap;
    static unordered_map<string, Object*>   m_ResourceCache;
    static unordered_map<Object*, string>   m_ReferenceCache;

    EnginePrivate() :
            m_pScene(nullptr) {

    }

    ~EnginePrivate() {
        delete m_pScene;

        m_Values.clear();

        m_pPlatform->destroy();
        delete m_pPlatform;
    }

    Scene                      *m_pScene;

    list<ISystem *>             m_Systems;

    static IFile               *m_pFile;

    string                      m_EntryLevel;

    static bool                 m_Game;

    static string               m_ApplicationPath;

    static string               m_ApplicationDir;

    static string               m_Organization;

    static string               m_Application;

    static IPlatformAdaptor    *m_pPlatform;

    static VariantMap           m_Values;
};

IFile *EnginePrivate::m_pFile   = nullptr;

bool                            EnginePrivate::m_Game = false;
StringMap                       EnginePrivate::m_IndexMap;
unordered_map<string, Object*>  EnginePrivate::m_ResourceCache;
unordered_map<Object*, string>  EnginePrivate::m_ReferenceCache;
VariantMap                      EnginePrivate::m_Values;
string                          EnginePrivate::m_ApplicationPath;
string                          EnginePrivate::m_ApplicationDir;
string                          EnginePrivate::m_Organization;
string                          EnginePrivate::m_Application;
IPlatformAdaptor               *EnginePrivate::m_pPlatform = nullptr;

typedef Vector4 Color;
/*!
    \class Engine
    \brief The Engine one of the central parts of Thunder Engine.
    \inmodule Engine

    The Engine class is one of the central parts of the Thunder Engine.
    This class is created first and removed last in your game.
    It is responsible for many basic functions, such as game cycle, management of game modules, loading and unloading of game resources, work with game settings.
*/

/*!
    \fn T *Engine::loadResource(const string &path)

    Returns an instance of type T for loading resource by the provided \a path.
    \note In case of resource was loaded previously this function will return the same instance.

    \sa unloadResource()
*/
/*!
    Constructs Engine.
    Creates necessary platform adapters, register basic component types and resource types.
*/
Engine::Engine(IFile *file, int, char **argv) :
        p_ptr(new EnginePrivate()) {
    PROFILER_MARKER;

    EnginePrivate::m_ApplicationPath = argv[0];
    Uri uri(EnginePrivate::m_ApplicationPath);
    EnginePrivate::m_ApplicationDir = uri.dir();
    EnginePrivate::m_Application = uri.baseName();

    p_ptr->m_pFile  = file;

#ifdef THUNDER_MOBILE
    p_ptr->m_pPlatform  = new MobileAdaptor(this);
#else
    p_ptr->m_pPlatform  = new DesktopAdaptor(this);
#endif

    Text::registerClassFactory(this);
    Texture::registerClassFactory(this);
    Material::registerClassFactory(this);
    Mesh::registerClassFactory(this);
    Atlas::registerClassFactory(this);
    Font::registerClassFactory(this);
    AnimationClip::registerClassFactory(this);
    RenderTexture::registerClassFactory(this);

    Scene::registerClassFactory(this);
    Actor::registerClassFactory(this);
    Component::registerClassFactory(this);
    Transform::registerClassFactory(this);
    Camera::registerClassFactory(this);

    MeshRender::registerClassFactory(this);
    TextRender::registerClassFactory(this);
    SpriteRender::registerClassFactory(this);
    DirectLight::registerClassFactory(this);
    PointLight::registerClassFactory(this);
    SpotLight::registerClassFactory(this);

    ParticleRender::registerClassFactory(this);
    ParticleEffect::registerClassFactory(this);

    AnimationStateMachine::registerClassFactory(this);
    AnimationController::registerClassFactory(this);

    Pipeline::registerClassFactory(this);

    NativeBehaviour::registerClassFactory(this);
    Renderable::registerClassFactory(this);
    BaseLight::registerClassFactory(this);

    ICommandBuffer::registerClassFactory(this);

    registerMetaType<Alignment>("Alignment");

    p_ptr->m_pScene = Engine::objectCreate<Scene>("Scene");
}
/*!
    Destructs Engine, related objects, registered object factories and platform adaptor.
*/
Engine::~Engine() {
    PROFILER_MARKER;

    delete p_ptr;
}
/*!
    Initializes all engine systems.
*/
bool Engine::init() {
    PROFILER_MARKER;

    bool result = p_ptr->m_pPlatform->init();

    Timer::init();
    Input::init(p_ptr->m_pPlatform);

    return result;
}
/*!
    Starts the main game cicle.
    Also this method loads the first level of your game.
*/
bool Engine::start() {
    PROFILER_MARKER;

    EnginePrivate::m_Game = true;

    p_ptr->m_pPlatform->start();

    reloadBundle();
    for(auto it : p_ptr->m_Systems) {
        if(!it->init()) {
            Log(Log::ERR) << "Failed to initialize system:" << it->name();
            p_ptr->m_pPlatform->stop();
            return false;
        }
    }

    string path = value(gEntry, "").toString();
    Actor *level = loadResource<Actor>(path);
    Log(Log::DBG) << "Level:" << path.c_str() << "loading...";
    if(level) {
        level->setParent(p_ptr->m_pScene);
    }

    Camera *component   = p_ptr->m_pScene->findChild<Camera *>();
    if(component == nullptr) {
        Log(Log::DBG) << "Camera not found creating new one.";
        Actor *camera = Engine::objectCreate<Actor>("ActiveCamera", p_ptr->m_pScene);
        camera->transform()->setPosition(Vector3(0.0f));
        component = camera->addComponent<Camera>();
    }
    Camera::setCurrent(component);

    resize();

#ifndef THUNDER_MOBILE
    while(p_ptr->m_pPlatform->isValid()) {
        update();
    }
    p_ptr->m_pPlatform->stop();
#endif
    return true;
}
/*!
    This method must be called each time when your game screen changes its size.
    \note Usually, this method calls internally and must not be called manually.
*/
void Engine::resize() {
    Camera *component = Camera::current();
    component->pipeline()->resize(p_ptr->m_pPlatform->screenWidth(), p_ptr->m_pPlatform->screenHeight());
    component->setRatio(float(p_ptr->m_pPlatform->screenWidth()) / float(p_ptr->m_pPlatform->screenHeight()));
}
/*!
    This method launches all your game modules responsible for processing all the game logic.
    It calls on each iteration of the game cycle.
    \note Usually, this method calls internally and must not be called manually.
*/
void Engine::update() {
    PROFILER_MARKER;

    Timer::update();
    // fixed update
    updateScene(p_ptr->m_pScene);

    for(auto it : p_ptr->m_Systems) {
        it->update(p_ptr->m_pScene);
    }
    p_ptr->m_pPlatform->update();
}
/*!
    Returns the value for setting \a key. If the setting doesn't exist, returns \a defaultValue.
*/
Variant Engine::value(const string &key, const Variant &defaultValue) {
    PROFILER_MARKER;

    auto it = EnginePrivate::m_Values.find(key);
    if(it != EnginePrivate::m_Values.end()) {
        return it->second;
    }
    return defaultValue;
}
/*!
    Sets the value of setting \a key to \a value. If the \a key already exists, the previous value is overwritten.
*/
void Engine::setValue(const string &key, const Variant &value) {
    PROFILER_MARKER;

    EnginePrivate::m_Values[key] = value;
}
/*!
    Returns an instance for loading resource by the provided \a path.
    \note In case of resource was loaded previously this function will return the same instance.

    \sa unloadResource()
*/
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
/*!
    Unloads a resource located along the \a path from memory.

    \sa loadResource()
*/
void Engine::unloadResource(const string &path) {
    PROFILER_MARKER;

    if(!path.empty()) {
        string uuid = path;
        {
            auto it = EnginePrivate::m_IndexMap.find(path);
            if(it != EnginePrivate::m_IndexMap.end()) {
                uuid = it->second;
            }
        }
        {
            auto it = EnginePrivate::m_ResourceCache.find(uuid);
            if(it != EnginePrivate::m_ResourceCache.end() && it->second) {
                delete it->second;

                EnginePrivate::m_ResourceCache.erase(it);
            }
        }
    }
}
/*!
    Register resource \a object by \a uuid path.

    \sa setResource()
*/
void Engine::setResource(Object *object, const string &uuid) {
    PROFILER_MARKER;

    EnginePrivate::m_ResourceCache[uuid] = object;
    EnginePrivate::m_ReferenceCache[object] = uuid;
}
/*!
    Returns resource path for the provided resource \a object.

    \sa setResource()
*/
string Engine::reference(Object *object) {
    PROFILER_MARKER;

    auto it = EnginePrivate::m_ReferenceCache.find(object);
    if(it != EnginePrivate::m_ReferenceCache.end()) {
        return it->second;
    }
    return string();
}
/*!
    This method reads the index file for the resource bundle.
    The index file helps to find required game resources.
*/
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

            EnginePrivate::m_Application = value(gProject, "").toString();
            EnginePrivate::m_Organization = value(gCompany, "").toString();
        }
    }
}
/*!
    Returns all loaded indices for game resources.
*/
StringMap Engine::indices() const {
    return EnginePrivate::m_IndexMap;
}
/*!
    Returns true if game started; otherwise returns false.
*/
bool Engine::isGameMode() {
    return EnginePrivate::m_Game;
}
/*!
    Set game \a flag to true if game started; otherwise set false.
*/
void Engine::setGameMode(bool flag) {
    EnginePrivate::m_Game = flag;
}
/*!
    Adds a game \a module to pool.
    This module will be used during update() method execution.

    Example:
    \code
    if(engine->init()) {
        engine->addModule(new RenderGL(engine));

        engine->start();
    }
    \endcode
*/
void Engine::addModule(IModule *module) {
    PROFILER_MARKER;
    if(module->types() & IModule::SYSTEM) {
        p_ptr->m_Systems.push_back(module->system());
    }
}
/*!
    Returns game Scene.
    \note The game can have only one scene. Scene is a root object, all map loads on this scene.
*/
Scene *Engine::scene() {
    PROFILER_MARKER;
    return p_ptr->m_pScene;
}
/*!
    Returns file system module.
*/
IFile *Engine::file() {
    PROFILER_MARKER;

    return EnginePrivate::m_pFile;
}
/*!
    Returns path to application binary directory.
*/
string Engine::locationAppDir() {
    PROFILER_MARKER;

    return EnginePrivate::m_ApplicationDir;
}
/*!
    Returns path to application config directory.
*/
string Engine::locationAppConfig() {
    PROFILER_MARKER;

    string result = EnginePrivate::m_pPlatform->locationLocalDir();
#ifndef THUNDER_MOBILE
    if(!EnginePrivate::m_Organization.empty()) {
        result  += "/" + EnginePrivate::m_Organization;
    }
    if(!EnginePrivate::m_Application.empty()) {
        result  += "/" + EnginePrivate::m_Application;
    }
#endif
    return result;
}
/*!
    Returns application name.
*/
string Engine::applicationName() const {
    PROFILER_MARKER;

    return EnginePrivate::m_Application;
}
/*!
    Returns organization name.
*/
string Engine::organizationName() const {
    PROFILER_MARKER;

    return EnginePrivate::m_Organization;
}
/*!
    This method launches your game logic.
    It calls on each iteration of the game cycle.
    \note Usually, this method calls internally and must not be called manually.
*/
void Engine::updateScene(Scene *scene) {
    PROFILER_MARKER;

    if(isGameMode()) {
        for(auto it : m_List) {
            NativeBehaviour *comp = dynamic_cast<NativeBehaviour *>(it);
            if(comp && comp->isEnable() && comp->actor()->scene() == scene) {
                if(!comp->isStarted()) {
                    comp->start();
                    comp->setStarted(true);
                }
                comp->update();
            }
        }
    }
}
