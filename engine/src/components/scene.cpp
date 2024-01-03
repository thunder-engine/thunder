#include "components/scene.h"

#include "components/world.h"

#include "resources/resource.h"

/*!
    \class Scene
    \brief The internal methods are marked as internal and are intended for use within the framework rather than by external code.
    \inmodule Components

    The Scene class serves as a container for actors and entities within the application, providing methods to interact with the world and manage the associated resource.
*/
Scene::Scene() :
        m_resource(nullptr),
        m_modified(false) {

}
/*!
    Returns the World to which the scene belongs.
*/
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
