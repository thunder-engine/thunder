#include "components/scene.h"

#include "components/actor.h"
#include "components/nativebehaviour.h"

#include "components/private/postprocessorsettings.h"

class ScenePrivate {
public:
    ScenePrivate() :
        m_Dirty(true),
        m_Update(false) {

    }

    bool m_Dirty;
    bool m_Update;

    PostProcessSettings m_FinalPostProcessSettings;
};

/*!
    \class Scene
    \brief A root object in the scene graph hierarchy.
    \inmodule Engine

    \note A scene object creating automatically by the engine.
    Only one Scene instance can be created in the game.
    A scene object must be set as a parent for other game hierarchies to show them on the screen.
    The main scene object can be retrieved using Engine::scene()
*/

Scene::Scene() :
    p_ptr(new ScenePrivate) {

}

Scene::~Scene() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    Returns in case of scene must be updated in the current frame; otherwise returns false.
*/
bool Scene::isToBeUpdated() {
    return p_ptr->m_Update;
}
/*!
    Sets an update \a flag.
*/
void Scene::setToBeUpdated(bool flag) {
    p_ptr->m_Update = flag;
}
/*!
    Returns true in case of scene is dirty and must be rendered; othewise returns false.
*/
bool Scene::isDirty() {
    return p_ptr->m_Dirty;
}
/*!
    Sets a dirty \a flag.
*/
void Scene::setDirty(bool flag) {
    p_ptr->m_Dirty = flag;
}
/*!
    \internal
    This method is used for the internal purposes and shouldn't be used externally.
*/
PostProcessSettings &Scene::finalPostProcessSettings() {
    return p_ptr->m_FinalPostProcessSettings;
}
