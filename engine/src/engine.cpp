#include "engine.h"

#include <stdint.h>

#include <log.h>
#include <file.h>

#include <objectsystem.h>
#include <bson.h>
#include <json.h>
#include <metatype.h>
#include <url.h>
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

#include "components/spline.h"

#ifdef THUNDER_MOBILE
    #include "adapters/mobileadaptor.h"
#else
    #include "adapters/desktopadaptor.h"
#endif

#include "resources/translator.h"

#include "systems/resourcesystem.h"
#include "systems/rendersystem.h"

#include "pipelinecontext.h"

namespace {
    static const char *gIndex("index");

    static const char *gVersion("version");
    static const char *gContent("content");
    static const char *gSettings("settings");

    static const char *gObjects("objects");

    static const char *gEntry(".entry");
    static const char *gCompany(".company");
    static const char *gProject(".project");

    static const char *gTransform("Transform");
}

#define INDEX_VERSION 2

class SystemRunner : public Runable {
public:
    explicit SystemRunner(System *system) :
            m_system(system) {

        setAutoDelete(false);
    }

    void run() override {
        m_system->processEvents();
    }

    System *m_system;
};

static bool m_game = false;

static VariantMap m_values;
static std::list<SystemRunner *> m_pool;
static std::list<System *> m_serial;

static std::list<NativeBehaviour *> m_behaviours;

static TString m_applicationPath;
static TString m_applicationDir;
static TString m_organization;
static TString m_application;

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
    \fn template<typename T> T *loadResource(const String &path)

    Returns an instance of type T for loading resource by the provided \a path.
    \note In case of resource was loaded previously this function will return the same instance.

    \sa unloadResource()
*/
/*!
    Constructs Engine.
    Using application \a path parameters creates necessary platform adapters, register basic component types and resource types.
*/
Engine::Engine(const char *path) {
    PROFILE_FUNCTION();

    std::locale::global(std::locale("C"));

    m_instance = this;

    addSystem(new ResourceSystem);
    m_applicationPath = path;
    Url url(m_applicationPath);
    m_applicationDir = url.absoluteDir();
    m_application = url.baseName();

    Url::declareMetaType();

    World::registerClassFactory(m_instance);
    Scene::registerClassFactory(m_instance);
    Actor::registerClassFactory(m_instance);
    Component::registerClassFactory(m_instance);
    Transform::registerClassFactory(m_instance);
    Camera::registerClassFactory(m_instance);

    Animator::registerClassFactory(m_instance);

    NativeBehaviour::registerClassFactory(m_instance);

    Armature::registerClassFactory(m_instance);

    PlayerInput::registerClassFactory(m_instance);

    Spline::registerClassFactory(m_instance);

    m_world = ObjectSystem::objectCreate<World>("World");

    uint32_t maxThreads = MAX(ThreadPool::optimalThreadCount() - 1, 1);
    if(maxThreads > 1) {
        m_threadPool = new ThreadPool;
        m_threadPool->setMaxThreads(maxThreads);
    } else {
        aWarning() << "Engine's Thread pool disabled.";
    }
}
/*!
    Destructs Engine, related objects, registered object factories and platform adaptor.
*/
Engine::~Engine() {
    PROFILE_FUNCTION();

    auto localSerial = m_serial;
    for(auto it : localSerial) {
        delete it;
    }

    auto localPool = m_pool;
    for(auto it : localPool) {
        delete it->m_system;
        delete it;
    }

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
    return setPlatformAdaptor(new MobileAdaptor);
#else
    return setPlatformAdaptor(new DesktopAdaptor);
#endif
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
        if(!it->m_system->init()) {
            aError() << "Failed to initialize system:" << it->m_system->name();
            return false;
        }
    }

    for(auto it : m_serial) {
        if(!it->init()) {
            aError() << "Failed to initialize system:" << it->name();
            return false;
        }
    }

    setGameMode(true);

    TString path = value(gEntry, "").toString();
    if(loadScene(path, false) == nullptr) {
        aError() << "Unable to load first scene:" << path;
        return false;
    }

    m_platform->loop();

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
        camera = nullptr;
        for(auto it : m_world->findChildren<Camera *>()) {
            if(it->isEnabled() && it->actor()->isEnabled()) { // Get first active Camera
                camera = it;
                break;
            }
        }

        if(camera == nullptr) {
            static Camera *reserveCamera = nullptr;
            if(reserveCamera == nullptr) {
                Actor *cameraActor = Engine::composeActor<Camera>("ReserveCamera", m_world);
                reserveCamera = cameraActor->getComponent<Camera>();
            }
            camera = reserveCamera;
            camera->actor()->setEnabled(true);
            camera->setEnabled(true);
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
            for(auto it : m_behaviours) {
                if(it->isEnabled()) {
                    World *world = it->world();

                    if(world == m_world) {
                        if(!it->isStarted()) {
                            it->start();
                            it->setStarted(true);
                        }
                        it->update();
                    }
                }
            }
        }

        m_world->setToBeUpdated(true);

        for(auto it : m_pool) {
            it->m_system->setActiveWorld(m_world);
            if(m_threadPool) {
                m_threadPool->start(it);
            } else {
                it->m_system->processEvents();
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
Variant Engine::value(const TString &key, const Variant &defaultValue) {
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
void Engine::setValue(const TString &key, const Variant &value) {
    PROFILE_FUNCTION();

    m_values[key] = value;
}
/*!
    Applies all unsaved settings.
*/
void Engine::syncValues() {
    PROFILE_FUNCTION();

    for(auto it : m_pool) {
        it->m_system->syncSettings();
    }
    for(auto it : m_serial) {
        it->syncSettings();
    }

    VariantMap filtered;
    for(auto &it : m_values) {
        if(it.first.front() != '.') {
            filtered[it.first] = it.second;
        }
    }

    m_platform->syncConfiguration(filtered);
}
/*!
    Returns an instance for loading resource by the provided \a path.
    \note In case of resource was loaded previously this function will return the same instance.

    \sa unloadResource()
*/
Resource *Engine::loadResource(const TString &path) {
    PROFILE_FUNCTION();

    return m_resourceSystem->loadResource(path);
}
/*!
    Returns an instance for loading resource by the provided \a path.
    The resource will be loaded asynchronously.
    This means you should check the state of resource before use it.
    \note In case of resource was loaded previously this function will return the same instance.

    \sa unloadResource()
*/
Resource *Engine::loadResourceAsync(const TString &path) {
    PROFILE_FUNCTION();

    return m_resourceSystem->loadResourceAsync(path);
}
/*!
    Forcely unloads the resource located along the \a path from memory.
    \warning After this call, the reference on the resource may become an invalid at any time and must not be used anymore.

    \sa loadResource()
*/
void Engine::unloadResource(const TString &path) {
    PROFILE_FUNCTION();

    if(!path.isEmpty()) {
        TString uuid = path;
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
void Engine::reloadResource(const TString &path) {
    PROFILE_FUNCTION();

    if(!path.isEmpty()) {
        TString uuid = path;
        Resource *resource = m_resourceSystem->resource(uuid);
        m_resourceSystem->reloadResource(resource, true);
    }
}
/*!
    Returns true if resource with \a path exists; otherwise returns false.
*/
bool Engine::isResourceExist(const TString &path) {
    return m_resourceSystem->isResourceExist(path);
}
/*!
    Replaces a current \a platform adaptor with new one;
    Returns true if replacement been succeeded; otherwise returns false.

    \note The previous platform adaptor will not be deleted.
*/
bool Engine::setPlatformAdaptor(PlatformAdaptor *platform) {
    m_platform = platform;

    bool result = m_platform->init();

    Input::init(m_platform);
    Timer::reset();

    return result;
}
/*!
    Returns resource path for the provided resource \a object.
*/
TString Engine::reference(Object *object) {
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

    ResourceSystem::Dictionary &indices = m_resourceSystem->indices();
    indices.clear();

    File fp(gIndex);
    if(fp.open(File::ReadOnly)) {
        Variant var = Json::load(TString(fp.readAll()));
        if(var.isValid()) {
            VariantMap root = var.toMap();

            int32_t version = root[gVersion].toInt();
            if(version == INDEX_VERSION) {
                for(auto &it : root[gContent].toMap()) {
                    VariantList item = it.second.toList();
                    auto i = item.begin();
                    TString path = i->toString();
                    i++;
                    TString type = i->toString();
                    i++;
                    TString md5 = i->toString();

                    uint32_t id = 0;
                    if(item.size() > 3) {
                        i++;
                        id = static_cast<uint32_t>(i->toInt());
                    }

                    indices[path] = {type, it.first, md5, id};
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
    PROFILE_FUNCTION();

    m_game = flag;

    for(auto it : m_pool) {
        it->m_system->reset();
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
            addSystem(reinterpret_cast<System *>(module->getObject(it.first.data())));
        }
    }
}
/*!
    \internal
*/
void Engine::addSystem(System *system) {
    PROFILE_FUNCTION();

    if(system->threadPolicy() == System::Pool) {
        m_pool.push_back(new SystemRunner(system));
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
    Removes a game \a system from pool.
*/
void Engine::removeSystem(System *system) {
    PROFILE_FUNCTION();

    m_serial.remove(system);

    for(auto it : m_pool) {
        if(it->m_system == system) {
            m_pool.remove(it);
            break;
        }
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
Scene *Engine::loadScene(const TString &path, bool additive) {
    PROFILE_FUNCTION();

    return m_world->loadScene(path, additive);
}
/*!
    Unloads the \a scene from the World.
*/
void Engine::unloadScene(Scene *scene) {
    PROFILE_FUNCTION();

    m_world->unloadScene(scene);
}
/*!
    Unloads all scenes from the World.
*/
void Engine::unloadAllScenes() {
    PROFILE_FUNCTION();

    m_world->unloadAll();
}
/*!
    Returns path to application binary directory.
*/
TString Engine::locationAppDir() {
    PROFILE_FUNCTION();

    return m_applicationDir;
}
/*!
    Returns path to application config directory.
*/
TString Engine::locationAppConfig() {
    PROFILE_FUNCTION();

    TString result = m_platform->locationLocalDir();
#ifndef THUNDER_MOBILE
    if(!m_organization.isEmpty()) {
        result += TString("/") + m_organization;
    }
    if(!m_application.isEmpty()) {
        result += TString("/") + m_application;
    }
#endif
    return result;
}
/*!
    Loads translation table with provided file \a name.
    This method generates the LanguageChange event for the Engine instance.
    An Engine instance will propagate the event to all top-level widgets, where reimplementation of event() can re-translate user-visible Strings.
    Returns true on success; otherwise returns false.
*/
bool Engine::loadTranslator(const TString &name) {
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
    Returns the translation text for the \a source String.
*/
TString Engine::translate(const TString &source) {
    PROFILE_FUNCTION();

    if(m_translator) {
        return m_translator->translate(source);
    }
    return source;
}
/*!
    Returns application name.
*/
TString Engine::applicationName() {
    PROFILE_FUNCTION();

    return m_application;
}
/*!
    Returns organization name.
*/
TString Engine::organizationName() {
    PROFILE_FUNCTION();

    return m_organization;
}
/*!
    Creates an Actor with \a name and attached \a component.
    Created Actor will be added to the hierarchy of \a parent.
    This method helps to create all dependencies for the \a component.
    \warning This method should be used only in Editor mode.
*/
Actor *Engine::composeActor(const TString &component, const TString &name, Object *parent) {
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
                } else {
                    comp->composeComponent();
                }
            }
        }

        if(actor->transform() == nullptr) {
            actor->addComponent(gTransform);
        }
    }
    return actor;
}

Object::ObjectList Engine::getAllObjectsByType(const TString &type) const {
    Object::ObjectList result = ObjectSystem::getAllObjectsByType(type);

    for(auto it : m_serial) {
        Object::ObjectList serial = it->getAllObjectsByType(type);
        result.insert(result.end(), serial.begin(), serial.end());
    }

    for(auto it : m_pool) {
        Object::ObjectList pool = it->m_system->getAllObjectsByType(type);
        result.insert(result.end(), pool.begin(), pool.end());
    }

    return result;
}

void Engine::addNativeBehaviour(NativeBehaviour *native) {
    m_behaviours.push_back(native);
}

void Engine::removeNativeBehaviour(NativeBehaviour *native) {
    m_behaviours.remove(native);
}
