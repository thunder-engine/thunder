#include "components/world.h"

#include "components/actor.h"
#include "components/scene.h"
#include "resources/map.h"

#include <algorithm>

/*!
    \class World
    \brief A root object in the scene graph hierarchy.
    \inmodule Components

    \note A scene object creating automatically by the engine.
    Only one World instance can be created in the game.
    A scene object must be set as a parent for other game hierarchies to show them on the screen.
    The main scene graph object can be retrieved using Engine::sceneGraph()
*/

World::World() :
        m_rayCastCallback(nullptr),
        m_rayCastSystem(nullptr),
        m_activeScene(nullptr),
        m_gameController(nullptr),
        m_update(false) {

}
/*!
    Returns in case of world is active and must be updated in the current frame; otherwise returns false.
*/
bool World::isActive() {
    return m_update;
}
/*!
    Sets an active \a flag.
    For active worlds engine launches the simulation.
*/
void World::setActive(bool flag) {
    m_update = flag;
}
/*!
    Create an empty new Scene at runtime with the given \a name.
*/
Scene *World::createScene(const TString &name) {
    Scene *result = Engine::objectCreate<Scene>(name, this);
    m_scenes.push_back(result);
    return result;
}
/*!
    Loads the scene stored in the .map files by the it's \a path.
    \note The previous scenes will be not unloaded in the case of an \a additive flag is true.
*/
Scene *World::loadScene(const TString &path, bool additive) {
    Map *map = Engine::loadResource<Map>(path);
    if(map) {
        Scene *scene = map->scene();
        if(scene) {
            if(additive) {
                scene->setParent(this);
            } else {
                for(auto it : m_scenes) {
                    if(it != scene) {
                        unloadScene(it);
                    }
                }
                scene->setParent(this);
            }
            m_scenes.push_back(scene);
            sceneLoaded();
            return scene;
        }
    }
    return nullptr;
}
/*!
    Unloads the \a scene from the World.
*/
void World::unloadScene(Scene *scene) {
    Map *map = scene->map();
    if(map) {
        m_scenes.remove(scene);
        scene->setParent(map);
        Engine::unloadResource(map);

        sceneUnloaded();
        if(m_activeScene == scene) {
            Scene *newScene = nullptr;
            if(!m_scenes.empty()) {
                newScene = m_scenes.front();
            }
            setActiveScene(newScene);
        }
    }
}
/*!
    Unloads all from the World.
*/
void World::unloadAll() {
    for(auto it : m_scenes) {
        Map *map = it->map();
        if(map) {
            it->setParent(map);
            Engine::unloadResource(map);
        }
    }
    m_scenes.clear();
    setActiveScene(nullptr);
}
/*!
    Returns an active Scene.

    There must always be one Scene marked as the active at the same time.
*/
Scene *World::activeScene() const {
    return m_activeScene;
}
/*!
    Sets the \a scene to be active.

    There must always be one Scene marked as the active at the same time.
*/
void World::setActiveScene(Scene *scene) {
    m_activeScene = scene;
    activeSceneChanged();
}
/*!
    Returns a game controller object.

    Game controller is abstract object respocible for various gameplay aspects.
*/
Object *World::gameController() const {
    return m_gameController;
}
/*!
    Sets the game \a controller.

    Game controller is abstract object respocible for various gameplay aspects.
*/
void World::setGameController(Object *controller) {
    m_gameController = controller;
}
/*!
    Casts a \a ray, of length \a maxDistance, against all colliders in the World.
    Returns true if the ray has a \a hit point with a Collider; otherwise returns false.
*/
bool World::rayCast(const Ray &ray, float maxDistance, Ray::Hit *hit) {
    if(m_rayCastCallback) {
        return m_rayCastCallback(m_rayCastSystem, this, ray, maxDistance, hit);
    }
    return false;
}
/*!
    Sets the raycast \a callback function.

    This function will be used to check intersections with in game geometry.
    In the most cases implemented in the physical engines.
    This callback is added by any physical \a system by the default.
*/
void World::setRayCastHandler(RayCastCallback callback, System *system) {
    m_rayCastCallback = callback;
    m_rayCastSystem = system;
}
/*!
    Returns list of all scenes.
*/
std::list<Scene *> &World::scenes() {
    return m_scenes;
}
/*!
    Emmits signal when scene has been loaded.
*/
void World::sceneLoaded() {
    emitSignal(_SIGNAL(sceneLoaded()));
}
/*!
    Emmits signal when scene has been unloaded.
*/
void World::sceneUnloaded() {
    emitSignal(_SIGNAL(sceneUnloaded()));
}
/*!
    Emmits signal when active scene has been changed.
*/
void World::activeSceneChanged() {
    emitSignal(_SIGNAL(activeSceneChanged()));
}
/*!
    Emmits signal when graph has been updated.
*/
void World::graphUpdated() {
    emitSignal(_SIGNAL(graphUpdated()));
}
/*!
    \internal
*/
void World::addChild(Object *child, int32_t position) {
    Object::addChild(child, position);

    Scene *scene = dynamic_cast<Scene *>(child);
    if(scene) {
        auto it = std::find(m_scenes.begin(), m_scenes.end(), scene);
        if(it == m_scenes.end()) {
            m_scenes.push_back(scene);
        }
    }
    if(m_activeScene == nullptr && scene) {
        setActiveScene(scene);
    }
}

