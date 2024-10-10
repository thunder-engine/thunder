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

#include "components/world.h"
#include "components/scene.h"
#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "components/armature.h"

#include "components/animator.h"
#include "components/playerinput.h"

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
#include "resources/translator.h"
#include "resources/particleeffect.h"
#include "resources/pipeline.h"
#include "resources/pose.h"
#include "resources/prefab.h"
#include "resources/map.h"
#include "resources/computebuffer.h"
#include "resources/computeshader.h"

#include "resources/meshgroup.h"

#include "resources/controlscheme.h"

#include "resources/tileset.h"
#include "resources/tilemap.h"

#include "systems/resourcesystem.h"
#include "systems/rendersystem.h"

#include "pipelinecontext.h"

#include "log.h"

namespace {
    static const char *gIndex("index");

    static const char *gVersion("version");
    static const char *gContent("content");
    static const char *gSettings("settings");

    static const char *gObjects("objects");

    static const char *gEntry(".entry");
    static const char *gRhi(".rhi");
    static const char *gCompany(".company");
    static const char *gProject(".project");

    static const char *gTransform("Transform");
}

#define INDEX_VERSION 2

static bool m_game = false;

static VariantMap m_values;
static std::list<System *> m_pool;
static std::list<System *> m_serial;

static std::string m_applicationPath;
static std::string m_applicationDir;
static std::string m_organization;
static std::string m_application;

static File *m_file = nullptr;
static ThreadPool *m_threadPool = nullptr;
static PlatformAdaptor *m_platform = nullptr;
static Translator *m_translator = nullptr;
static World *m_world = nullptr;
static ResourceSystem *m_resourceSystem = nullptr;
static RenderSystem *m_renderSystem = nullptr;
static Engine *m_instance = nullptr;

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
Engine::Engine(File *file, const char *path) {
    PROFILE_FUNCTION();

    std::locale::global(std::locale("C"));

    m_instance = this;

    addSystem(new ResourceSystem);
    m_applicationPath = path;
    Uri uri(m_applicationPath);
    m_applicationDir = uri.dir();
    m_application = uri.baseName();

    m_file = file;

    // The order is critical for the import
    Resource::registerClassFactory(m_resourceSystem);

    Text::registerClassFactory(m_resourceSystem);
    Texture::registerClassFactory(m_resourceSystem);
    Material::registerClassFactory(m_resourceSystem);
    Mesh::registerClassFactory(m_resourceSystem);
    MeshGroup::registerClassFactory(m_resourceSystem);
    Sprite::registerClassFactory(m_resourceSystem);
    Font::registerClassFactory(m_resourceSystem);
    AnimationClip::registerClassFactory(m_resourceSystem);
    RenderTarget::registerClassFactory(m_resourceSystem);

    Translator::registerClassFactory(m_resourceSystem);
    Pose::registerSuper(m_resourceSystem);

    Prefab::registerClassFactory(m_resourceSystem);
    Map::registerClassFactory(m_resourceSystem);

    TileSet::registerClassFactory(m_resourceSystem);
    TileMap::registerClassFactory(m_resourceSystem);

    ParticleEffect::registerClassFactory(m_resourceSystem);

    AnimationStateMachine::registerSuper(m_resourceSystem);

    Pipeline::registerClassFactory(m_resourceSystem);

    ComputeBuffer::registerClassFactory(m_resourceSystem);
    ComputeShader::registerClassFactory(m_resourceSystem);

    World::registerClassFactory(m_instance);
    Scene::registerClassFactory(m_instance);
    Actor::registerClassFactory(m_instance);
    Component::registerClassFactory(m_instance);
    Transform::registerClassFactory(m_instance);
    Camera::registerClassFactory(m_instance);

    Animator::registerClassFactory(m_instance);

    NativeBehaviour::registerClassFactory(m_instance);

    Armature::registerClassFactory(m_instance);

    ControlScheme::registerClassFactory(m_resourceSystem);
    PlayerInput::registerClassFactory(m_instance);

    m_world = ObjectSystem::objectCreate<World>("World");
}
/*!
    Destructs Engine, related objects, registered object factories and platform adaptor.
*/
Engine::~Engine() {
    PROFILE_FUNCTION();

    delete m_threadPool;

    if(m_platform) {
        m_platform->destroy();
        delete m_platform;
    }
}
/*!
    Initializes all engine systems. Returns true if successful; otherwise returns false.
*/
bool Engine::init() {
    PROFILE_FUNCTION();

#ifdef THUNDER_MOBILE
    m_platform = new MobileAdaptor;
#else
    m_platform = new DesktopAdaptor(value(gRhi, "").toString());
#endif
    bool result = m_platform->init();

    Timer::reset();
    Input::init(m_platform);

    uint32_t maxThreads = MAX(ThreadPool::optimalThreadCount() - 1, 1);
    if(maxThreads > 1) {
        m_threadPool = new ThreadPool;
        m_threadPool->setMaxThreads(maxThreads);
    } else {
        aWarning() << "Engine's Thread pool disabled.";
    }

    return result;
}
/*!
    Starts the main game cycle.
    Also this method loads the first level of your game.
    Returns true if successful; otherwise returns false.
*/
bool Engine::start() {
    PROFILE_FUNCTION();

    m_platform->start();

    for(auto it : m_pool) {
        if(!it->init()) {
            aError() << "Failed to initialize system:" << it->name().c_str();
            m_platform->stop();
            return false;
        }
    }

    for(auto it : m_serial) {
        if(!it->init()) {
            aError() << "Failed to initialize system:" << it->name().c_str();
            m_platform->stop();
            return false;
        }
    }

    setGameMode(true);

    std::string path = value(gEntry, "").toString();
    if(loadScene(path, false) == nullptr) {
        aError() << "Unable to load first scene:" << path.c_str();
        m_platform->stop();
        return false;
    }

    Camera *component = m_world->findChild<Camera *>();
    if(component == nullptr) {
        aDebug() << "Camera not found creating a new one.";
        Actor *camera = Engine::composeActor("Camera", "ActiveCamera", m_world);
        camera->transform()->setPosition(Vector3(0.0f));
    }

#ifndef THUNDER_MOBILE
    while(m_platform->isValid()) {
        Timer::update();
        update();
    }
    m_platform->stop();
#endif
    return true;
}
/*!
    This method launches all your game modules responsible for processing all the game logic.
    It calls on each iteration of the game cycle.
    \note Usually, this method calls internally and must not be called manually.
*/
void Engine::update() {
    PROFILE_FUNCTION();

    // Active camera check
    Camera *camera = Camera::current();
    if(camera == nullptr || !camera->isEnabled() || !camera->actor()->isEnabled()) {
        for(auto it : m_world->findChildren<Camera *>()) {
            if(it->isEnabled() && it->actor()->isEnabled()) { // Get first active Camera
                camera = it;
                break;
            }
        }
        Camera::setCurrent(camera);
    }

    // Update screen size
    if(camera) {
        camera->setRatio(float(m_platform->screenWidth()) / float(m_platform->screenHeight()));
    }

    if(m_renderSystem) {
        PipelineContext *pipeline = m_renderSystem->pipelineContext();
        if(pipeline) {
            pipeline->resize(m_platform->screenWidth(), m_platform->screenHeight());
        }
    }

    // Process game cycle
    try {
        m_instance->processEvents();

        if(isGameMode()) {
            for(auto it : m_instance->m_objectList) {
                NativeBehaviour *comp = dynamic_cast<NativeBehaviour *>(it);
                if(comp && comp->isEnabled() && comp->world() == m_world) {
                    if(!comp->isStarted()) {
                        comp->start();
                        comp->setStarted(true);
                    }
                    comp->update();
                }
            }
        }

        m_world->setToBeUpdated(true);

        for(auto it : m_pool) {
            it->setActiveWorld(m_world);
            if(m_threadPool) {
                m_threadPool->start(*it);
            } else {
                it->processEvents();
            }
        }
        for(auto it : m_serial) {
            it->setActiveWorld(m_world);
            it->processEvents();
        }
        m_threadPool->waitForDone();

        m_world->setToBeUpdated(false);

        m_platform->update();

    } catch(...) {
        aError() << "Unable to process game cycle";
    }
}
/*!
    \internal
*/
bool Engine::event(Event *event) {
    switch(event->type()) {
    case Event::LanguageChange: {
        for(auto it : m_objectList) {
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
Variant Engine::value(const std::string &key, const Variant &defaultValue) {
    PROFILE_FUNCTION();

    auto it = m_values.find(key);
    if(it != m_values.end()) {
        return it->second;
    }
    return defaultValue;
}
/*!
    Sets the value of setting \a key to \a value. If the \a key already exists, the previous value will be overwritten.
*/
void Engine::setValue(const std::string &key, const Variant &value) {
    PROFILE_FUNCTION();

    m_values[key] = value;
}
/*!
    Applies all unsaved settings.
*/
void Engine::syncValues() {
    PROFILE_FUNCTION();

    for(auto it : m_pool) {
        it->syncSettings();
    }
    for(auto it : m_serial) {
        it->syncSettings();
    }

    m_platform->syncConfiguration(m_values);
}
/*!
    Returns an instance for loading resource by the provided \a path.
    \note In case of resource was loaded previously this function will return the same instance.

    \sa unloadResource()
*/
Object *Engine::loadResource(const std::string &path) {
    PROFILE_FUNCTION();

    return m_resourceSystem->loadResource(path);
}
/*!
    Forcely unloads the resource located along the \a path from memory.
    \warning After this call, the reference on the resource may become an invalid at any time and must not be used anymore.

    \sa loadResource()
*/
void Engine::unloadResource(const std::string &path) {
    PROFILE_FUNCTION();

    if(!path.empty()) {
        std::string uuid = path;
        unloadResource(m_resourceSystem->resource(uuid));
    }
}
/*!
    Forcely unloads the \a resource from memory.
    \warning After this call, the reference on the resource may become an invalid at any time and must not be used anymore.

    \sa loadResource()
*/
void Engine::unloadResource(Resource *resource) {
    PROFILE_FUNCTION();

    if(resource) {
        m_resourceSystem->unloadResource(resource, true);
    }
}
/*!
    Reloads the resource located along the \a path.

    \sa loadResource()
*/
void Engine::reloadResource(const std::string &path) {
    PROFILE_FUNCTION();

    if(!path.empty()) {
        std::string uuid = path;
        Resource *resource = m_resourceSystem->resource(uuid);
        m_resourceSystem->reloadResource(resource, true);
    }
}
/*!
    Returns true if resource with \a path exists; otherwise returns false.
*/
bool Engine::isResourceExist(const std::string &path) {
    return m_resourceSystem->isResourceExist(path);
}
/*!
    Register resource \a object by \a uuid path.

    \sa setResource()
*/
void Engine::setResource(Object *object, const std::string &uuid) {
    PROFILE_FUNCTION();

    m_resourceSystem->setResource(static_cast<Resource *>(object), uuid);
}
/*!
    Replaces a current \a platform adaptor with new one;
    \note The previous one will not be deleted.
*/
void Engine::setPlatformAdaptor(PlatformAdaptor *platform) {
    m_platform = platform;
}
/*!
    Returns resource path for the provided resource \a object.

    \sa setResource()
*/
std::string Engine::reference(Object *object) {
    PROFILE_FUNCTION();

    return m_resourceSystem->reference(static_cast<Resource *>(object));
}
/*!
    This method reads the index file for the resource bundle.
    The index file helps to find required game resources.
    Returns true in case of success; otherwise returns false.
*/
bool Engine::reloadBundle() {
    PROFILE_FUNCTION();
    ResourceSystem::DictionaryMap &indices = m_resourceSystem->indices();
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

        Variant var = Json::load(std::string(data.begin(), data.end()));
        if(var.isValid()) {
            VariantMap root = var.toMap();

            int32_t version = root[gVersion].toInt();
            if(version == INDEX_VERSION) {
                for(auto &it : root[gContent].toMap()) {
                    VariantList item = it.second.toList();
                    auto i = item.begin();
                    std::string path = i->toString();
                    i++;
                    std::string type = i->toString();
                    indices[path] = std::pair<std::string, std::string>(type, it.first);
                }

                for(auto &it : root[gSettings].toMap()) {
                    setValue(it.first, it.second);
                }

                m_application = value(gProject, "").toString();
                m_organization = value(gCompany, "").toString();

                return true;
            }
        }
    }
    return false;
}
/*!
    Returns the resource management system which can be used in external modules.
*/
ResourceSystem *Engine::resourceSystem() {
    return m_resourceSystem;
}
/*!
    Returns the render system which can be used in external modules.
*/
RenderSystem *Engine::renderSystem() {
    return m_renderSystem;
}
/*!
    Returns true if game started; otherwise returns false.
*/
bool Engine::isGameMode() {
    return m_game;
}
/*!
    Set game \a flag to true if game started; otherwise set false.
*/
void Engine::setGameMode(bool flag) {
    m_game = flag;

    for(auto it : m_pool) {
        it->reset();
    }

    for(auto it : m_serial) {
        it->reset();
    }
}
/*!
    Adds a game \a module to pool.
    This module will be used during update() method execution.

    Example:
    \code
    if(engine->init()) {
        Engine::addModule(new RenderGL(engine));

        engine->start();
    }
    \endcode
*/
void Engine::addModule(Module *module) {
    PROFILE_FUNCTION();
    VariantMap metaInfo = Json::load(module->metaInfo()).toMap();
    for(auto &it : metaInfo[gObjects].toMap()) {
        if(it.second.toString() == "system" || it.second.toString() == "render") {
            addSystem(reinterpret_cast<System *>(module->getObject(it.first.c_str())));
        }
    }
}
/*!
    \internal
*/
void Engine::addSystem(System *system) {
    if(system->threadPolicy() == System::Pool) {
        m_pool.push_back(system);
    } else {
        m_serial.push_back(system);
    }

    if(dynamic_cast<RenderSystem *>(system) != nullptr) {
        m_renderSystem = static_cast<RenderSystem *>(system);
    } else if(dynamic_cast<ResourceSystem *>(system) != nullptr) {
        m_resourceSystem = static_cast<ResourceSystem *>(system);
    }
}
/*!
    Returns game World.
    \note The game can have only one scene graph. World is a root object, all map loads on this World.
*/
World *Engine::world() {
    PROFILE_FUNCTION();

    return m_world;
}
/*!
    Loads the scene stored in the .map files by the it's \a path to the Engine.
    \note The previous scenes will be not unloaded in the case of an \a additive flag is true.
*/
Scene *Engine::loadScene(const std::string &path, bool additive) {
    return m_world->loadScene(path, additive);
}
/*!
    Unloads the \a scene from the World.
*/
void Engine::unloadScene(Scene *scene) {
    m_world->unloadScene(scene);
}
/*!
    Unloads all scenes from the World.
*/
void Engine::unloadAllScenes() {
    m_world->unloadAll();
}
/*!
    Returns file system module.
*/
File *Engine::file() {
    PROFILE_FUNCTION();

    return m_file;
}
/*!
    Returns path to application binary directory.
*/
std::string Engine::locationAppDir() {
    PROFILE_FUNCTION();

    return m_applicationDir;
}
/*!
    Returns path to application config directory.
*/
std::string Engine::locationAppConfig() {
    PROFILE_FUNCTION();

    std::string result = m_platform->locationLocalDir();
#ifndef THUNDER_MOBILE
    if(!m_organization.empty()) {
        result  += "/" + m_organization;
    }
    if(!m_application.empty()) {
        result  += "/" + m_application;
    }
#endif
    return result;
}
/*!
    Loads translation table with provided file \a name.
    This method generates the LanguageChange event for the Engine instance.
    An Engine instance will propagate the event to all top-level widgets, where reimplementation of event() can re-translate user-visible std::strings.
    Returns true on success; otherwise returns false.
*/
bool Engine::loadTranslator(const std::string &name) {
    PROFILE_FUNCTION();

    if(m_translator) {
        m_resourceSystem->unloadResource(m_translator);
    }

    m_translator = Engine::loadResource<Translator>(name);
    if(m_translator) {
        m_instance->postEvent(new Event(Event::LanguageChange));
        return true;
    }
    return false;
}
/*!
    Returns the translation text for the \a source std::string.
*/
std::string Engine::translate(const std::string &source) {
    PROFILE_FUNCTION();

    if(m_translator) {
        return m_translator->translate(source);
    }
    return source;
}
/*!
    Returns application name.
*/
std::string Engine::applicationName() {
    PROFILE_FUNCTION();

    return m_application;
}
/*!
    Returns organization name.
*/
std::string Engine::organizationName() {
    PROFILE_FUNCTION();

    return m_organization;
}
/*!
    Creates an Actor with \a name and attached \a component.
    Created Actor will be added to the hierarchy of \a parent.
    This method helps to create all dependencies for the \a component.
    \warning This method should be used only in Editor mode.
*/
Actor *Engine::composeActor(const std::string &component, const std::string &name, Object *parent) {
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

Object::ObjectList Engine::getAllObjectsByType(const std::string &type) const {
    Object::ObjectList result = ObjectSystem::getAllObjectsByType(type);

    for(auto it : m_serial) {
        Object::ObjectList serial = it->getAllObjectsByType(type);
        result.insert(result.end(), serial.begin(), serial.end());
    }

    for(auto it : m_pool) {
        Object::ObjectList pool = it->getAllObjectsByType(type);
        result.insert(result.end(), pool.begin(), pool.end());
    }

    return result;
}
