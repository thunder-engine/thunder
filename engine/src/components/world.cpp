#include "components/world.h"

#include "components/actor.h"
#include "components/scene.h"
#include "resources/map.h"

#include "components/private/postprocessorsettings.h"

/*!
    \class World
    \brief A root object in the scene graph hierarchy.
    \inmodule Engine

    \note A scene object creating automatically by the engine.
    Only one World instance can be created in the game.
    A scene object must be set as a parent for other game hierarchies to show them on the screen.
    The main scene graph object can be retrieved using Engine::sceneGraph()
*/

World::World() :
    m_rayCastCallback(nullptr),
    m_rayCastSystem(nullptr),
    m_activeScene(nullptr),
    m_dirty(true),
    m_update(false) {

}
/*!
    Returns in case of scene must be updated in the current frame; otherwise returns false.
*/
bool World::isToBeUpdated() {
    return m_update;
}
/*!
    Sets an update \a flag.
*/
void World::setToBeUpdated(bool flag) {
    m_update = flag;
    if(!m_update && m_dirty) {
        emitSignal(_SIGNAL(graphUpdated()));
        m_dirty = false;
    }
}
/*!
    Marks World as dirty. Mainly used to detect scene graph configuration changes.
*/
void World::makeDirty() {
    m_dirty = true;
}
/*!
    Create an empty new Scene at runtime with the given \a name.
*/
Scene *World::createScene(const string &name) {
    return Engine::objectCreate<Scene>(name, this);
}
/*!
    Loads the scene stored in the .map files by the it's \a path.
    \note The previous scenes will be not unloaded in the case of an \a additive flag is true.
*/
Scene *World::loadScene(const string &path, bool additive) {
    Map *map = Engine::loadResource<Map>(path);
    if(map) {
        Scene *scene = map->scene();
        if(scene) {
            if(additive) {
                scene->setParent(this);
            } else {
                for(auto it : getChildren()) {
                    unloadScene(dynamic_cast<Scene *>(it));
                }
                scene->setParent(this);
            }
            emitSignal(_SIGNAL(sceneLoaded()));
            return scene;
        }
    }
    return nullptr;
}
/*!
    Unloads the \a scene from the World.
*/
void World::unloadScene(Scene *scene) {
    Resource *map = scene->resource();
    if(map) {
        Engine::unloadResource(map);
        emitSignal(_SIGNAL(sceneUnloaded()));
        if(m_activeScene == scene) {
            Scene *newScene = nullptr;
            for(auto it : getChildren()) {
                newScene = dynamic_cast<Scene *>(it);
                if(newScene != nullptr) {
                    break;
                }
            }
            setActiveScene(newScene);
        }
    }
}
/*!
    Unloads all from the World.
*/
void World::unloadAll() {
    Object::ObjectList copyList(getChildren());
    for(auto it : copyList) {
        delete it;
    }
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
    emitSignal(_SIGNAL(activeSceneChanged()));
}
/*!
    Casts a ray, from point \a origin, in direction \a direction, of length \a maxDistance, against all colliders in the World.
    Returns true if the ray has a \a hit with a Collider; otherwise returns false.
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
    \internal
*/
void World::addChild(Object *child, int32_t position) {
    Object::addChild(child, position);
    if(m_activeScene == nullptr && dynamic_cast<Scene *>(child)) {
        setActiveScene(static_cast<Scene *>(child));
    }
}

