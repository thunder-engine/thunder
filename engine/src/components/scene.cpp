#include "components/scene.h"

#include "components/world.h"

/*!
    \class Scene
    \brief The internal methods are marked as internal and are intended for use within the framework rather than by external code.
    \inmodule Components

    The Scene class serves as a container for actors and entities within the application, providing methods to interact with the world and manage the associated resource.
*/
Scene::Scene() :
        m_map(nullptr),
        m_modified(false) {

}

Scene::Scene(const Scene &origin) :
        m_groups(origin.m_groups),
        m_map(origin.m_map),
        m_modified(origin.m_modified) {

}
/*!
    Returns the World to which the scene belongs.
*/
World *Scene::world() const {
    return static_cast<World *>(parent());
}
/*!
    Adds \a object to \a group.
*/
void Scene::addToGroup(Object *object, const TString &group) {
    addToGroupByHash(object, static_cast<uint32_t>(Mathf::hashString(group)));
}
/*!
    Adds \a object to a group with specific \a hash.
*/
void Scene::addToGroupByHash(Object *object, uint32_t hash) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_groups[hash].push_back(object);
}
/*!
    Removes \a object from \a group.
*/
void Scene::removeFromGroup(Object *object, const TString &group) {
    removeFromGroupByHash(object, static_cast<uint32_t>(Mathf::hashString(group)));
}
/*!
    Removes \a object from a group with specific \a hash.
*/
void Scene::removeFromGroupByHash(Object *object, uint32_t hash) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_groups.find(hash);
    if(it != m_groups.end()) {
        it->second.remove(object);
    }
}
/*!
    Returns a list of objects in \a group.
*/
Object::ObjectList &Scene::getObjectsInGroup(const TString &group) {
    return getObjectsInGroupByHash(static_cast<uint32_t>(Mathf::hashString(group)));
}
/*!
    Returns a list of objects from a group with specific \a hash.
*/
Object::ObjectList &Scene::getObjectsInGroupByHash(uint32_t hash) {
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_groups[hash];
}
/*!
    \internal
*/
Map *Scene::map() const {
    return m_map;
}
/*!
    \internal
*/
void Scene::setMap(Map *map) {
    m_map = map;
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
