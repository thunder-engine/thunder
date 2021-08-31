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

Scene::Scene() :
    p_ptr(new ScenePrivate) {

}

Scene::~Scene() {
    delete p_ptr;
}

bool Scene::isToBeUpdated() {
    return p_ptr->m_Update;
}
void Scene::setToBeUpdated(bool flag) {
    p_ptr->m_Update = flag;
}

bool Scene::isDirty() {
    return p_ptr->m_Dirty;
}
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
