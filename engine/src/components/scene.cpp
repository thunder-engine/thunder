#include "components/scene.h"

#include "components/actor.h"
#include "resources/resource.h"

Scene::Scene() :
    m_resource(nullptr),
    m_modified(false) {

}

Scene::~Scene() {
    m_resource = nullptr;
}
/*!
    \internal
*/
Resource *Scene::resource() const {
    return m_resource;
}
/*!
    \internal
*/
void Scene::setResource(Resource *resource) {
    m_resource = resource;
}
/*!
    \internal
*/
bool Scene::isModified() const {
    return m_modified;
}
/*!
    \internal
*/
void Scene::setModified(bool flag) {
    m_modified = flag;
}
