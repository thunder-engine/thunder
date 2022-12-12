#include "components/scene.h"

#include "components/world.h"

#include "resources/resource.h"

Scene::Scene() :
    m_resource(nullptr),
    m_modified(false) {

}

World *Scene::world() const {
    return static_cast<World *>(parent());
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
