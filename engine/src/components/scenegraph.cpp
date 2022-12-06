#include "components/scenegraph.h"

#include "components/actor.h"
#include "components/scene.h"
#include "resources/map.h"

#include "components/private/postprocessorsettings.h"

/*!
    \class SceneGraph
    \brief A root object in the scene graph hierarchy.
    \inmodule Engine

    \note A scene object creating automatically by the engine.
    Only one SceneGraph instance can be created in the game.
    A scene object must be set as a parent for other game hierarchies to show them on the screen.
    The main scene graph object can be retrieved using Engine::sceneGraph()
*/

SceneGraph::SceneGraph() :
    m_activeScene(nullptr),
    m_finalPostProcessSettings(new PostProcessSettings),
    m_dirty(true),
    m_update(false) {

}

SceneGraph::~SceneGraph() {

}
/*!
    Returns in case of scene must be updated in the current frame; otherwise returns false.
*/
bool SceneGraph::isToBeUpdated() {
    return m_update;
}
/*!
    Sets an update \a flag.
*/
void SceneGraph::setToBeUpdated(bool flag) {
    m_update = flag;
}
/*!
    Create an empty new Scene at runtime with the given \a name.
*/
Scene *SceneGraph::createScene(const string &name) {
    return Engine::objectCreate<Scene>(name, this);
}
/*!
    Loads the scene stored in the .map files by the it's \a path.
    \note The previous scenes will be not unloaded in the case of an \a additive flag is true.
*/
Scene *SceneGraph::loadScene(const string &path, bool additive) {
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
    Unloads the \a scene from the SceneGraph.
*/
void SceneGraph::unloadScene(Scene *scene) {
    Resource *map = dynamic_cast<Resource *>(scene->resource());
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
    Returns an active Scene.

    There must always be one Scene marked as the active at the same time.
*/
Scene *SceneGraph::activeScene() const {
    return m_activeScene;
}
/*!
    Sets the \a scene to be active.

    There must always be one Scene marked as the active at the same time.
*/
void SceneGraph::setActiveScene(Scene *scene) {
    m_activeScene = scene;
    emitSignal(_SIGNAL(activeSceneChanged()));
}

/*!
    \internal
    This method is used for the internal purposes and shouldn't be used externally.
*/
PostProcessSettings &SceneGraph::finalPostProcessSettings() {
    return *m_finalPostProcessSettings;
}
/*!
    \internal
*/
void SceneGraph::addChild(Object *child, int32_t position) {
    Object::addChild(child, position);
    if(m_activeScene == nullptr && dynamic_cast<Scene *>(child)) {
        setActiveScene(static_cast<Scene *>(child));
    }
}

