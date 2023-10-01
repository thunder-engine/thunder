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
#include "commandbuffer.h"

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

class EnginePrivate {
public:
    EnginePrivate() {

        locale::global(locale("C"));
    }

    ~EnginePrivate() {
        m_values.clear();

        if(m_platform) {
            m_platform->destroy();
            delete m_platform;
        }

        //for(auto it : m_pool) {
        //    delete it;
        //}
        m_pool.clear();

        //for(auto it : m_serial) {
        //    delete it;
        //}
        m_serial.clear();
    }

    static list<System *>    m_pool;
    static list<System *>    m_serial;

    static World            *m_world;

    static Engine           *m_instance;

    static File             *m_file;

    string                   m_entryLevel;

    static bool              m_game;

    static string            m_applicationPath;

    static string            m_applicationDir;

    static string            m_organization;

    static string            m_application;

    static PlatformAdaptor  *m_platform;

    static VariantMap        m_values;

    ThreadPool               m_threadPool;

    static ResourceSystem   *m_resourceSystem;

    static RenderSystem     *m_renderSystem;

    static Translator       *m_translator;
};

File            *EnginePrivate::m_file = nullptr;

bool             EnginePrivate::m_game = false;
VariantMap       EnginePrivate::m_values;
string           EnginePrivate::m_applicationPath;
string           EnginePrivate::m_applicationDir;
string           EnginePrivate::m_organization;
string           EnginePrivate::m_application;
PlatformAdaptor *EnginePrivate::m_platform = nullptr;
World           *EnginePrivate::m_world = nullptr;
ResourceSystem  *EnginePrivate::m_resourceSystem = nullptr;
RenderSystem    *EnginePrivate::m_renderSystem = nullptr;
Translator      *EnginePrivate::m_translator = nullptr;
Engine          *EnginePrivate::m_instance = nullptr;

list<System *>  EnginePrivate::m_pool;
list<System *>  EnginePrivate::m_serial;

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

    EnginePrivate::m_instance = this;

    addSystem(new ResourceSystem);
    EnginePrivate::m_applicationPath = path;
    Uri uri(EnginePrivate::m_applicationPath);
    EnginePrivate::m_applicationDir = uri.dir();
    EnginePrivate::m_application = uri.baseName();

    EnginePrivate::m_file = file;

    // The order is critical for the import
    Resource::registerClassFactory(EnginePrivate::m_resourceSystem);

    Text::registerClassFactory(EnginePrivate::m_resourceSystem);
    Texture::registerClassFactory(EnginePrivate::m_resourceSystem);
    Material::registerClassFactory(EnginePrivate::m_resourceSystem);
    Mesh::registerClassFactory(EnginePrivate::m_resourceSystem);
    MeshGroup::registerClassFactory(EnginePrivate::m_resourceSystem);
    Sprite::registerClassFactory(EnginePrivate::m_resourceSystem);
    Font::registerClassFactory(EnginePrivate::m_resourceSystem);
    AnimationClip::registerClassFactory(EnginePrivate::m_resourceSystem);
    RenderTarget::registerClassFactory(EnginePrivate::m_resourceSystem);

    Translator::registerClassFactory(EnginePrivate::m_resourceSystem);
    Pose::registerSuper(EnginePrivate::m_resourceSystem);

    Prefab::registerClassFactory(EnginePrivate::m_resourceSystem);
    Map::registerClassFactory(EnginePrivate::m_resourceSystem);

    TileSet::registerClassFactory(EnginePrivate::m_resourceSystem);
    TileMap::registerClassFactory(EnginePrivate::m_resourceSystem);

    ParticleEffect::registerSuper(EnginePrivate::m_resourceSystem);

    AnimationStateMachine::registerSuper(EnginePrivate::m_resourceSystem);

    Pipeline::registerClassFactory(EnginePrivate::m_resourceSystem);

    ComputeBuffer::registerClassFactory(EnginePrivate::m_resourceSystem);
    ComputeShader::registerClassFactory(EnginePrivate::m_resourceSystem);

    World::registerClassFactory(this);
    Scene::registerClassFactory(this);
    Actor::registerClassFactory(this);
    Component::registerClassFactory(this);
    Transform::registerClassFactory(this);
    Camera::registerClassFactory(this);

    Animator::registerClassFactory(this);

    NativeBehaviour::registerClassFactory(this);

    Armature::registerClassFactory(this);

    ControlScheme::registerClassFactory(EnginePrivate::m_resourceSystem);
    PlayerInput::registerClassFactory(this);

    EnginePrivate::m_world = Engine::objectCreate<World>("World");
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
    EnginePrivate::m_platform = new MobileAdaptor;
#else
    EnginePrivate::m_platform = new DesktopAdaptor(value(gRhi, "").toString());
#endif
    bool result = EnginePrivate::m_platform->init();

    Timer::reset();
    Input::init(EnginePrivate::m_platform);

    p_ptr->m_threadPool.setMaxThreads(MAX(ThreadPool::optimalThreadCount() - 1, 1));

    return result;
}
/*!
    Starts the main game cycle.
    Also this method loads the first level of your game.
    Returns true if successful; otherwise returns false.
*/
bool Engine::start() {
    PROFILE_FUNCTION();

    EnginePrivate::m_platform->start();

    for(auto it : EnginePrivate::m_pool) {
        if(!it->init()) {
            aError() << "Failed to initialize system:" << it->name().c_str();
            EnginePrivate::m_platform->stop();
            return false;
        }
    }
    for(auto it : EnginePrivate::m_serial) {
        if(!it->init()) {
            aError() << "Failed to initialize system:" << it->name().c_str();
            EnginePrivate::m_platform->stop();
            return false;
        }
    }

    EnginePrivate::m_game = true;

    string path = value(gEntry, "").toString();
    if(loadScene(path, false) == nullptr) {
        aError() << "Unable to load" << path.c_str();
        EnginePrivate::m_platform->stop();
        return false;
    }

    Camera *component = EnginePrivate::m_world->findChild<Camera *>();
    if(component == nullptr) {
        aDebug() << "Camera not found creating a new one.";
        Actor *camera = Engine::composeActor("Camera", "ActiveCamera", EnginePrivate::m_world);
        camera->transform()->setPosition(Vector3(0.0f));
    }

    resize();

#ifndef THUNDER_MOBILE
    while(EnginePrivate::m_platform->isValid()) {
        Timer::update();
        update();
    }
    EnginePrivate::m_platform->stop();
#endif
    return true;
}
/*!
    This method must be called each time when your game screen changes its size.
    \note Usually, this method calls internally and must not be called manually.
*/
void Engine::resize() {
    PROFILE_FUNCTION();

    PipelineContext *pipeline = EnginePrivate::m_renderSystem->pipelineContext();
    if(pipeline) {
        pipeline->resize(EnginePrivate::m_platform->screenWidth(), EnginePrivate::m_platform->screenHeight());
    }

    Camera *component = Camera::current();
    if(component) {
        component->setRatio(float(EnginePrivate::m_platform->screenWidth()) / float(EnginePrivate::m_platform->screenHeight()));
    }
}
/*!
    This method launches all your game modules responsible for processing all the game logic.
    It calls on each iteration of the game cycle.
    \note Usually, this method calls internally and must not be called manually.
*/
void Engine::update() {
    PROFILE_FUNCTION();

    Camera *camera = Camera::current();
    if(camera == nullptr || !camera->isEnabled() || !camera->actor()->isEnabled()) {
        for(auto it : EnginePrivate::m_world->findChildren<Camera *>()) {
            if(it->isEnabled() && it->actor()->isEnabled()) { // Get first active Camera
                camera = it;
                break;
            }
        }
        Camera::setCurrent(camera);
    }
    resize();

    processEvents();

    EnginePrivate::m_world->setToBeUpdated(true);

    for(auto it : EnginePrivate::m_pool) {
        it->setActiveGraph(EnginePrivate::m_world);
        p_ptr->m_threadPool.start(*it);
    }
    for(auto it : EnginePrivate::m_serial) {
        it->setActiveGraph(EnginePrivate::m_world);
        it->processEvents();
    }
    p_ptr->m_threadPool.waitForDone();

    EnginePrivate::m_world->setToBeUpdated(false);

    EnginePrivate::m_platform->update();
}
/*!
    \internal
*/
void Engine::processEvents() {
    PROFILE_FUNCTION();

    try {
        ObjectSystem::processEvents();

        if(isGameMode()) {
            for(auto it : m_objectList) {
                NativeBehaviour *comp = dynamic_cast<NativeBehaviour *>(it);
                if(comp && comp->isEnabled() && comp->world() == EnginePrivate::m_world) {
                    if(!comp->isStarted()) {
                        comp->start();
                        comp->setStarted(true);
                    }
                    comp->update();
                }
            }
        }
    } catch(...) {
        aError() << "Unable to proceess game cycle";
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
Variant Engine::value(const string &key, const Variant &defaultValue) {
    PROFILE_FUNCTION();

    auto it = EnginePrivate::m_values.find(key);
    if(it != EnginePrivate::m_values.end()) {
        return it->second;
    }
    return defaultValue;
}
/*!
    Sets the value of setting \a key to \a value. If the \a key already exists, the previous value will be overwritten.
*/
void Engine::setValue(const string &key, const Variant &value) {
    PROFILE_FUNCTION();

    EnginePrivate::m_values[key] = value;
}
/*!
    Applies all unsaved settings.
*/
void Engine::syncValues() {
    PROFILE_FUNCTION();

    for(auto it : EnginePrivate::m_pool) {
        it->syncSettings();
    }
    for(auto it : EnginePrivate::m_serial) {
        it->syncSettings();
    }

    EnginePrivate::m_platform->syncConfiguration(EnginePrivate::m_values);
}
/*!
    Returns an instance for loading resource by the provided \a path.
    \note In case of resource was loaded previously this function will return the same instance.

    \sa unloadResource()
*/
Object *Engine::loadResource(const string &path) {
    PROFILE_FUNCTION();

    return EnginePrivate::m_resourceSystem->loadResource(path);
}
/*!
    Forcely unloads the resource located along the \a path from memory.
    \warning After this call, the reference on the resource may become an invalid at any time and must not be used anymore.

    \sa loadResource()
*/
void Engine::unloadResource(const string &path) {
    PROFILE_FUNCTION();

    if(!path.empty()) {
        string uuid = path;
        unloadResource(EnginePrivate::m_resourceSystem->resource(uuid));
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
        EnginePrivate::m_resourceSystem->unloadResource(resource, true);
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
        Resource *resource = EnginePrivate::m_resourceSystem->resource(uuid);
        EnginePrivate::m_resourceSystem->reloadResource(resource, true);
    }
}
/*!
    Returns true if resource with \a path exists; otherwise returns false.
*/
bool Engine::isResourceExist(const string &path) {
    return EnginePrivate::m_resourceSystem->isResourceExist(path);
}
/*!
    Register resource \a object by \a uuid path.

    \sa setResource()
*/
void Engine::setResource(Object *object, const string &uuid) {
    PROFILE_FUNCTION();

    EnginePrivate::m_resourceSystem->setResource(static_cast<Resource *>(object), uuid);
}
/*!
    Replaces a current \a platform adaptor with new one;
    \note The previous one will not be deleted.
*/
void Engine::setPlatformAdaptor(PlatformAdaptor *platform) {
    EnginePrivate::m_platform = platform;
}
/*!
    Returns resource path for the provided resource \a object.

    \sa setResource()
*/
string Engine::reference(Object *object) {
    PROFILE_FUNCTION();

    return EnginePrivate::m_resourceSystem->reference(static_cast<Resource *>(object));
}
/*!
    This method reads the index file for the resource bundle.
    The index file helps to find required game resources.
    Returns true in case of success; otherwise returns false.
*/
bool Engine::reloadBundle() {
    PROFILE_FUNCTION();
    ResourceSystem::DictionaryMap &indices = EnginePrivate::m_resourceSystem->indices();
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
                    EnginePrivate::m_values[it.first] = it.second;
                }

                EnginePrivate::m_application = value(gProject, "").toString();
                EnginePrivate::m_organization = value(gCompany, "").toString();

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
    return EnginePrivate::m_resourceSystem;
}
/*!
    Returns the render system which can be used in external modules.
*/
RenderSystem *Engine::renderSystem() {
    return EnginePrivate::m_renderSystem;
}
/*!
    Returns true if game started; otherwise returns false.
*/
bool Engine::isGameMode() {
    return EnginePrivate::m_game;
}
/*!
    Set game \a flag to true if game started; otherwise set false.
*/
void Engine::setGameMode(bool flag) {
    EnginePrivate::m_game = flag;
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
        EnginePrivate::m_pool.push_back(system);
    } else {
        EnginePrivate::m_serial.push_back(system);
    }

    if(dynamic_cast<RenderSystem *>(system) != nullptr) {
        EnginePrivate::m_renderSystem = static_cast<RenderSystem *>(system);
    } else if(dynamic_cast<ResourceSystem *>(system) != nullptr) {
        EnginePrivate::m_resourceSystem = static_cast<ResourceSystem *>(system);
    }
}
/*!
    Returns game World.
    \note The game can have only one scene graph. World is a root object, all map loads on this World.
*/
World *Engine::world() {
    PROFILE_FUNCTION();

    return EnginePrivate::m_world;
}
/*!
    Loads the scene stored in the .map files by the it's \a path to the Engine.
    \note The previous scenes will be not unloaded in the case of an \a additive flag is true.
*/
Scene *Engine::loadScene(const string &path, bool additive) {
    return EnginePrivate::m_world->loadScene(path, additive);
}
/*!
    Unloads the \a scene from the World.
*/
void Engine::unloadScene(Scene *scene) {
    EnginePrivate::m_world->unloadScene(scene);
}
/*!
    Returns file system module.
*/
File *Engine::file() {
    PROFILE_FUNCTION();

    return EnginePrivate::m_file;
}
/*!
    Returns path to application binary directory.
*/
string Engine::locationAppDir() {
    PROFILE_FUNCTION();

    return EnginePrivate::m_applicationDir;
}
/*!
    Returns path to application config directory.
*/
string Engine::locationAppConfig() {
    PROFILE_FUNCTION();

    string result = EnginePrivate::m_platform->locationLocalDir();
#ifndef THUNDER_MOBILE
    if(!EnginePrivate::m_organization.empty()) {
        result  += "/" + EnginePrivate::m_organization;
    }
    if(!EnginePrivate::m_application.empty()) {
        result  += "/" + EnginePrivate::m_application;
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

    if(EnginePrivate::m_translator) {
        EnginePrivate::m_resourceSystem->unloadResource(EnginePrivate::m_translator);
    }

    EnginePrivate::m_translator = Engine::loadResource<Translator>(name);
    if(EnginePrivate::m_translator) {
        EnginePrivate::m_instance->postEvent(new Event(Event::LanguageChange));
        return true;
    }
    return false;
}
/*!
    Returns the translation text for the \a source string.
*/
string Engine::translate(const string &source) {
    PROFILE_FUNCTION();

    if(EnginePrivate::m_translator) {
        return EnginePrivate::m_translator->translate(source);
    }
    return source;
}
/*!
    Returns application name.
*/
string Engine::applicationName() {
    PROFILE_FUNCTION();

    return EnginePrivate::m_application;
}
/*!
    Returns organization name.
*/
string Engine::organizationName() {
    PROFILE_FUNCTION();

    return EnginePrivate::m_organization;
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

Object::ObjectList Engine::getAllObjectsByType(const string &type) const {
    Object::ObjectList result = ObjectSystem::getAllObjectsByType(type);

    for(auto it : EnginePrivate::m_serial) {
        Object::ObjectList serial = it->getAllObjectsByType(type);
        result.insert(result.end(), serial.begin(), serial.end());
    }

    for(auto it : EnginePrivate::m_pool) {
        Object::ObjectList pool = it->getAllObjectsByType(type);
        result.insert(result.end(), pool.begin(), pool.end());
    }

    return result;
}
