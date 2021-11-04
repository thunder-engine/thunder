#include "engine.h"

#include <stdint.h>
#include <sstream>

#include <log.h>
#include <file.h>

#include <objectsystem.h>
#include <bson.h>
#include <json.h>
#include <metatype.h>
#include <uri.h>
#include <threadpool.h>

#include "module.h"
#include "system.h"
#include "timer.h"
#include "input.h"

#include "components/scene.h"
#include "components/chunk.h"
#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "components/armature.h"

#include "components/animationcontroller.h"

#ifdef THUNDER_MOBILE
    #include "adapters/mobileadaptor.h"
#else
    #include "adapters/desktopadaptor.h"
#endif
#include "resources/text.h"
#include "resources/texture.h"
#include "resources/rendertarget.h"
#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/font.h"
#include "resources/animationclip.h"
#include "resources/animationstatemachine.h"
#include "resources/pipeline.h"
#include "resources/translator.h"
#include "resources/particleeffect.h"
#include "resources/pose.h"
#include "resources/prefab.h"
#include "resources/map.h"

#include "systems/resourcesystem.h"

#include "log.h"

namespace {
    static const char *gIndex("index");

    static const char *gVersion("version");
    static const char *gContent("content");
    static const char *gSettings("settings");

    static const char *gSystems("systems");

    static const char *gEntry(".entry");
    static const char *gCompany(".company");
    static const char *gProject(".project");

    static const char *gTransform("Transform");
}

#define INDEX_VERSION 2

class EnginePrivate {
public:
    EnginePrivate() {

        locale::global(locale("C"));
    }

    ~EnginePrivate() {
        m_Values.clear();

        if(m_Platform) {
            m_Platform->destroy();
            delete m_Platform;
        }

        //for(auto it : m_Pool) {
        //    delete it;
        //}
        m_Pool.clear();

        //for(auto it : m_Serial) {
        //    delete it;
        //}
        m_Serial.clear();
    }

    static list<System *>    m_Pool;
    static list<System *>    m_Serial;

    static Scene            *m_Scene;

    static Engine           *m_Instance;

    static File             *m_File;

    string                   m_EntryLevel;

    static bool              m_Game;

    static string            m_ApplicationPath;

    static string            m_ApplicationDir;

    static string            m_Organization;

    static string            m_Application;

    static PlatformAdaptor  *m_Platform;

    static VariantMap        m_Values;

    ThreadPool               m_ThreadPool;

    static ResourceSystem   *m_pResourceSystem;

    static Translator       *m_Translator;
};

File            *EnginePrivate::m_File = nullptr;

bool             EnginePrivate::m_Game = false;
VariantMap       EnginePrivate::m_Values;
string           EnginePrivate::m_ApplicationPath;
string           EnginePrivate::m_ApplicationDir;
string           EnginePrivate::m_Organization;
string           EnginePrivate::m_Application;
PlatformAdaptor *EnginePrivate::m_Platform = nullptr;
Scene           *EnginePrivate::m_Scene = nullptr;
ResourceSystem  *EnginePrivate::m_pResourceSystem = nullptr;
Translator      *EnginePrivate::m_Translator = nullptr;
Engine          *EnginePrivate::m_Instance = nullptr;

list<System *>  EnginePrivate::m_Pool;
list<System *>  EnginePrivate::m_Serial;

typedef Vector4 Color;

/*!
    \group Thunder Engine

    \title Thunder Engine Software Developer Kit
*/

/*!
    \module Engine

    \title Thunder Engine Software Developer Kit

    \brief Contains base game management classes.
*/

/*!
    \class Engine
    \brief The Engine one of the central parts of Thunder Engine.
    \inmodule Engine

    The Engine class is one of the central parts of the Thunder Engine.
    This class is created first and removed last in your game.
    It is responsible for many basic functions, such as game cycle, management of game modules, loading and unloading of game resources, work with game settings.
*/

/*!
    \fn template<typename T> T *loadResource(const std::string &path)

    Returns an instance of type T for loading resource by the provided \a path.
    \note In case of resource was loaded previously this function will return the same instance.

    \sa unloadResource()
*/
/*!
    Constructs Engine.
    Using \a file and \a path parameters creates necessary platform adapters, register basic component types and resource types.
*/
Engine::Engine(File *file, const char *path) :
        p_ptr(new EnginePrivate()) {
    PROFILE_FUNCTION();

    EnginePrivate::m_Instance = this;

    EnginePrivate::m_pResourceSystem = new ResourceSystem;
    EnginePrivate::m_Serial.push_back(p_ptr->m_pResourceSystem);
    EnginePrivate::m_ApplicationPath = path;
    Uri uri(EnginePrivate::m_ApplicationPath);
    EnginePrivate::m_ApplicationDir = uri.dir();
    EnginePrivate::m_Application = uri.baseName();

    EnginePrivate::m_File = file;

    // The order is critical for the import
    Resource::registerClassFactory(p_ptr->m_pResourceSystem);

    Text::registerClassFactory(p_ptr->m_pResourceSystem);
    Texture::registerClassFactory(p_ptr->m_pResourceSystem);
    Material::registerClassFactory(p_ptr->m_pResourceSystem);
    Mesh::registerSuper(p_ptr->m_pResourceSystem);
    Sprite::registerClassFactory(p_ptr->m_pResourceSystem);
    Font::registerClassFactory(p_ptr->m_pResourceSystem);
    AnimationClip::registerClassFactory(p_ptr->m_pResourceSystem);
    RenderTarget::registerClassFactory(p_ptr->m_pResourceSystem);

    Pipeline::registerClassFactory(p_ptr->m_pResourceSystem);
    Translator::registerClassFactory(p_ptr->m_pResourceSystem);
    Pose::registerSuper(p_ptr->m_pResourceSystem);

    Prefab::registerClassFactory(p_ptr->m_pResourceSystem);
    Map::registerClassFactory(p_ptr->m_pResourceSystem);

    ParticleEffect::registerSuper(p_ptr->m_pResourceSystem);

    AnimationStateMachine::registerSuper(p_ptr->m_pResourceSystem);

    Scene::registerClassFactory(this);
    Chunk::registerClassFactory(this);
    Actor::registerClassFactory(this);
    Component::registerClassFactory(this);
    Transform::registerClassFactory(this);
    Camera::registerClassFactory(this);

    AnimationController::registerClassFactory(this);

    NativeBehaviour::registerClassFactory(this);

    Armature::registerClassFactory(this);

    EnginePrivate::m_Scene = Engine::objectCreate<Scene>("Scene");
}
/*!
    Destructs Engine, related objects, registered object factories and platform adaptor.
*/
Engine::~Engine() {
    PROFILE_FUNCTION();

    deleteAllObjects();

    delete p_ptr;
}
/*!
    Initializes all engine systems. Returns true if successful; otherwise returns false.
*/
bool Engine::init() {
    PROFILE_FUNCTION();

#ifdef THUNDER_MOBILE
    EnginePrivate::m_Platform = new MobileAdaptor(this);
#else
    EnginePrivate::m_Platform = new DesktopAdaptor(this);
#endif
    bool result = EnginePrivate::m_Platform->init();

    Timer::reset();
    Input::init(EnginePrivate::m_Platform);

    p_ptr->m_ThreadPool.setMaxThreads(MAX(ThreadPool::optimalThreadCount() - 1, 1));

    return result;
}
/*!
    Starts the main game cycle.
    Also this method loads the first level of your game.
    Returns true if successful; otherwise returns false.
*/
bool Engine::start() {
    PROFILE_FUNCTION();

    p_ptr->m_Platform->start();

    for(auto it : EnginePrivate::m_Pool) {
        if(!it->init()) {
            Log(Log::ERR) << "Failed to initialize system:" << it->name();
            p_ptr->m_Platform->stop();
            return false;
        }
    }
    for(auto it : EnginePrivate::m_Serial) {
        if(!it->init()) {
            Log(Log::ERR) << "Failed to initialize system:" << it->name();
            p_ptr->m_Platform->stop();
            return false;
        }
    }

    EnginePrivate::m_Game = true;

    string path = value(gEntry, "").toString();
    if(loadSceneChunk(path, false) == nullptr) {
        Log(Log::ERR) << "Unable to load" << path.c_str();
        p_ptr->m_Platform->stop();
        return false;
    }

    Camera *component = EnginePrivate::m_Scene->findChild<Camera *>();
    if(component == nullptr) {
        Log(Log::DBG) << "Camera not found creating a new one.";
        Actor *camera = Engine::composeActor("Camera", "ActiveCamera", EnginePrivate::m_Scene);
        camera->transform()->setPosition(Vector3(0.0f));
    }

    resize();

#ifndef THUNDER_MOBILE
    while(p_ptr->m_Platform->isValid()) {
        update();
    }
    p_ptr->m_Platform->stop();
#endif
    return true;
}
/*!
    This method must be called each time when your game screen changes its size.
    \note Usually, this method calls internally and must not be called manually.
*/
void Engine::resize() {
    PROFILE_FUNCTION();

    Camera *component = Camera::current();
    if(component) {
        component->pipeline()->resize(p_ptr->m_Platform->screenWidth(), p_ptr->m_Platform->screenHeight());
        component->setRatio(float(p_ptr->m_Platform->screenWidth()) / float(p_ptr->m_Platform->screenHeight()));
    }
}
/*!
    This method launches all your game modules responsible for processing all the game logic.
    It calls on each iteration of the game cycle.
    \note Usually, this method calls internally and must not be called manually.
*/
void Engine::update() {
    PROFILE_FUNCTION();

    Timer::update();

    Camera *camera = Camera::current();
    if(camera == nullptr || !camera->isEnabled() || !camera->actor()->isEnabled()) {
        for(auto it : EnginePrivate::m_Scene->findChildren<Camera *>()) {
            if(it->isEnabled() && it->actor()->isEnabled()) { // Get first active Camera
                camera = it;
                break;
            }
        }
        Camera::setCurrent(camera);
    }
    resize();

    processEvents();

    EnginePrivate::m_Scene->setToBeUpdated(true);

    for(auto it : EnginePrivate::m_Pool) {
        it->setActiveScene(EnginePrivate::m_Scene);
        p_ptr->m_ThreadPool.start(*it);
    }
    for(auto it : EnginePrivate::m_Serial) {
        it->setActiveScene(EnginePrivate::m_Scene);
        it->processEvents();
    }
    p_ptr->m_ThreadPool.waitForDone();

    EnginePrivate::m_Scene->setToBeUpdated(false);

    p_ptr->m_Platform->update();
}
/*!
    \internal
*/
void Engine::processEvents() {
    PROFILE_FUNCTION();

    ObjectSystem::processEvents();

    if(isGameMode()) {
        for(auto it : m_ObjectList) {
            NativeBehaviour *comp = dynamic_cast<NativeBehaviour *>(it);
            if(comp && comp->isEnabled() && comp->actor() && comp->actor()->scene() == EnginePrivate::m_Scene) {
                if(!comp->isStarted()) {
                    comp->start();
                    comp->setStarted(true);
                }
                comp->update();
            }
        }
    }
}
/*!
    \internal
*/
bool Engine::event(Event *event) {
    switch(event->type()) {
    case Event::LanguageChange: {
        for(auto it : m_ObjectList) {
            it->event(event);
        }
    } break;
    default: break;
    }
    return false;
}
/*!
    Returns the value for setting \a key. If the setting doesn't exist, returns \a defaultValue.
*/
Variant Engine::value(const string &key, const Variant &defaultValue) {
    PROFILE_FUNCTION();

    auto it = EnginePrivate::m_Values.find(key);
    if(it != EnginePrivate::m_Values.end()) {
        return it->second;
    }
    return defaultValue;
}
/*!
    Sets the value of setting \a key to \a value. If the \a key already exists, the previous value will be overwritten.
*/
void Engine::setValue(const string &key, const Variant &value) {
    PROFILE_FUNCTION();

    EnginePrivate::m_Values[key] = value;
}
/*!
    Applies all unsaved settings.
*/
void Engine::syncValues() {
    PROFILE_FUNCTION();

    for(auto it : EnginePrivate::m_Pool) {
        it->syncSettings();
    }
    for(auto it : EnginePrivate::m_Serial) {
        it->syncSettings();
    }

    EnginePrivate::m_Platform->syncConfiguration(EnginePrivate::m_Values);
}
/*!
    Returns an instance for loading resource by the provided \a path.
    \note In case of resource was loaded previously this function will return the same instance.

    \sa unloadResource()
*/
Object *Engine::loadResource(const string &path) {
    PROFILE_FUNCTION();

    return EnginePrivate::m_pResourceSystem->loadResource(path);
}
/*!
    Force unloads the resource located along the \a path from memory.
    \warning After this call, the reference on the resource may become an invalid at any time and must not be used anymore.

    \sa loadResource()
*/
void Engine::unloadResource(const string &path) {
    PROFILE_FUNCTION();

    if(!path.empty()) {
        string uuid = path;
        Resource *resource = EnginePrivate::m_pResourceSystem->resource(uuid);
        EnginePrivate::m_pResourceSystem->unloadResource(resource, true);
    }
}
/*!
    Reloads the resource located along the \a path.

    \sa loadResource()
*/
void Engine::reloadResource(const string &path) {
    PROFILE_FUNCTION();

    if(!path.empty()) {
        string uuid = path;
        Resource *resource = EnginePrivate::m_pResourceSystem->resource(uuid);
        EnginePrivate::m_pResourceSystem->reloadResource(resource, true);
    }
}
/*!
    Returns true if resource with \a path exists; otherwise returns false.
*/
bool Engine::isResourceExist(const string &path) {
    return EnginePrivate::m_pResourceSystem->isResourceExist(path);
}
/*!
    Register resource \a object by \a uuid path.

    \sa setResource()
*/
void Engine::setResource(Object *object, const string &uuid) {
    PROFILE_FUNCTION();

    EnginePrivate::m_pResourceSystem->setResource(static_cast<Resource *>(object), uuid);
}
/*!
    Replaces a current \a platform adaptor with new one;
    \note The previous one will not be deleted.
*/
void Engine::setPlatformAdaptor(PlatformAdaptor *platform) {
    EnginePrivate::m_Platform = platform;
}
/*!
    Returns resource path for the provided resource \a object.

    \sa setResource()
*/
string Engine::reference(Object *object) {
    PROFILE_FUNCTION();

    return EnginePrivate::m_pResourceSystem->reference(static_cast<Resource *>(object));
}
/*!
    This method reads the index file for the resource bundle.
    The index file helps to find required game resources.
    Returns true in case of success; otherwise returns false.
*/
bool Engine::reloadBundle() {
    PROFILE_FUNCTION();
    ResourceSystem::DictionaryMap &indices = EnginePrivate::m_pResourceSystem->indices();
    indices.clear();

    File *file = Engine::file();
    _FILE *fp = file->fopen(gIndex, "r");
    if(fp) {
        ByteArray data;
        data.resize(file->fsize(fp));
        if(data.empty()) {
            return false;
        }
        file->fread(&data[0], data.size(), 1, fp);
        file->fclose(fp);

        Variant var = Json::load(string(data.begin(), data.end()));
        if(var.isValid()) {
            VariantMap root = var.toMap();

            int32_t version = root[gVersion].toInt();
            if(version == INDEX_VERSION) {
                for(auto &it : root[gContent].toMap()) {
                    VariantList item = it.second.toList();
                    auto i = item.begin();
                    string path = i->toString();
                    i++;
                    string type = i->toString();
                    indices[path] = pair<string, string>(type, it.first);
                }

                for(auto &it : root[gSettings].toMap()) {
                    EnginePrivate::m_Values[it.first] = it.second;
                }

                EnginePrivate::m_Application = value(gProject, "").toString();
                EnginePrivate::m_Organization = value(gCompany, "").toString();

                return true;
            }
        }
    }
    return false;
}
/*!
    Returns the resource management system which can be used in external modules.
*/
System *Engine::resourceSystem() {
    return EnginePrivate::m_pResourceSystem;
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
void Engine::addModule(Module *module) {
    PROFILE_FUNCTION();
    VariantMap metaInfo = Json::load(module->metaInfo()).toMap();
    for(auto &it : metaInfo[gSystems].toList()) {
        System *system = module->system(it.toString().c_str());
        if(system->threadPolicy() == System::Pool) {
            EnginePrivate::m_Pool.push_back(system);
        } else {
            EnginePrivate::m_Serial.push_back(system);
        }
    }
}
/*!
    Returns game Scene.
    \note The game can have only one scene. Scene is a root object, all map loads on this scene.
*/
Scene *Engine::scene() {
    PROFILE_FUNCTION();

    return EnginePrivate::m_Scene;
}
/*!
    Loads the scene chunk stored in the .map files by the it's \a path to the Engine.
    \note The previous chunks will be not unloaded in the case of an \a additive flag is true.
*/
Chunk *Engine::loadSceneChunk(const string &path, bool additive) {
    Map *map = loadResource<Map>(path);
    if(map) {
        Chunk *chunk = map->chunk();
        if(chunk) {
            if(additive) {
                chunk->setParent(EnginePrivate::m_Scene);
            } else {
                for(auto it : EnginePrivate::m_Scene->getChildren()) {
                    unloadSceneChunk(dynamic_cast<Chunk *>(it));
                }
                chunk->setParent(EnginePrivate::m_Scene);
            }

            return chunk;
        }
    }
    return nullptr;
}

void Engine::unloadSceneChunk(Chunk *chunk) {
    Resource *map = dynamic_cast<Resource *>(chunk->resource());
    if(map) {
        EnginePrivate::m_pResourceSystem->unloadResource(map);
    }
}
/*!
    Returns file system module.
*/
File *Engine::file() {
    PROFILE_FUNCTION();

    return EnginePrivate::m_File;
}
/*!
    Returns path to application binary directory.
*/
string Engine::locationAppDir() {
    PROFILE_FUNCTION();

    return EnginePrivate::m_ApplicationDir;
}
/*!
    Returns path to application config directory.
*/
string Engine::locationAppConfig() {
    PROFILE_FUNCTION();

    string result = EnginePrivate::m_Platform->locationLocalDir();
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
    Loads translation table with provided file \a name.
    This method generates the LanguageChange event for the Engine instance.
    An Engine instance will propagate the event to all top-level widgets, where reimplementation of event() can re-translate user-visible strings.
    Returns true on success; otherwise returns false.
*/
bool Engine::loadTranslator(const string &name) {
    PROFILE_FUNCTION();

    if(EnginePrivate::m_Translator) {
        EnginePrivate::m_pResourceSystem->unloadResource(EnginePrivate::m_Translator);
    }

    EnginePrivate::m_Translator = Engine::loadResource<Translator>(name);
    if(EnginePrivate::m_Translator) {
        EnginePrivate::m_Instance->postEvent(new Event(Event::LanguageChange));
        return true;
    }
    return false;
}
/*!
    Returns the translation text for the \a source string.
*/
string Engine::translate(const string &source) {
    PROFILE_FUNCTION();

    if(EnginePrivate::m_Translator) {
        return EnginePrivate::m_Translator->translate(source);
    }
    return source;
}
/*!
    Returns application name.
*/
string Engine::applicationName() {
    PROFILE_FUNCTION();

    return EnginePrivate::m_Application;
}
/*!
    Returns organization name.
*/
string Engine::organizationName() {
    PROFILE_FUNCTION();

    return EnginePrivate::m_Organization;
}
/*!
    Creates an Actor with \a name and attached \a component.
    Created Actor will be added to the hierarchy of \a parent.
    This method helps to create all dependencies for the \a component.
    \warning This method should be used only in Editor mode.
*/
Actor *Engine::composeActor(const string &component, const string &name, Object *parent) {
    Actor *actor = Engine::objectCreate<Actor>(name, parent);
    if(actor) {
        Object *object = Engine::objectCreate(component, component, actor);
        Component *comp = dynamic_cast<Component *>(object);
        if(comp) {
            FactoryPair *pair = metaFactory(component);
            if(pair) {
                System *system = dynamic_cast<System *>(pair->second);
                if(system) {
                    system->composeComponent(comp);
                }
            }
        }

        if(actor->transform() == nullptr) {
            actor->addComponent(gTransform);
        }
    }
    return actor;
}
