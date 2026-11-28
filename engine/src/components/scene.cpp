/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "components/scene.h"

#include "components/world.h"
#include "components/component.h"

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

Scene::~Scene() {
    for(auto &group : m_groups) {
        for(auto it : group.second) {
            Component *component = dynamic_cast<Component *>(it);
            if(component) {
                component->tags().clear();
            }
        }
    }
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
