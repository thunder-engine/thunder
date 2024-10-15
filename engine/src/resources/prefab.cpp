#include <resources/prefab.h>

#include <components/actor.h>

namespace  {
    const char *gActor("Actor");
};

/*!
    \class Prefab
    \brief A small piece of objects hierarchy which can be placed on the scene.
    \inmodule Resources
*/

Prefab::Prefab() :
        m_actor(nullptr) {

}

Prefab::~Prefab() {

}
/*!
    Returns prototype Actor which will should be instanced
*/
Actor *Prefab::actor() const {
    if(m_actor == nullptr) {
        auto &children = getChildren();
        if(!children.empty()) {
            m_actor = dynamic_cast<Actor *>(children.front());
        }
    }
    return m_actor;
}
/*!
    \internal
*/
void Prefab::setActor(Actor *actor) {
    m_dictionary.clear();

    m_actor = actor;
    if(m_actor) {
        m_actor->setParent(this);
    }
}
/*!
    Returns true if prefab contains an object with provided \a uuid
*/
bool Prefab::contains(uint32_t uuid) {
    if(m_dictionary.empty()) {
        makeCache(m_actor);
    }
    return m_dictionary.find(uuid) != m_dictionary.end();
}
/*!
    Returns a prototype of object with provided \a uuid
*/
Object *Prefab::protoObject(uint32_t uuid) {
    if(m_dictionary.empty()) {
        makeCache(m_actor);
    }

    auto it = m_dictionary.find(uuid);
    if(it != m_dictionary.end()) {
        return it->second;
    }
    return nullptr;
}
/*!
    Compares with prefab and returns a list of abset \a objects in cloned list
*/
Prefab::ConstObjectList Prefab::absentObjects(const ConstObjectList &objects) {
    if(m_dictionary.empty()) {
        makeCache(m_actor);
    }

    ConstObjectList temp;
    for(auto it : m_dictionary) {
        temp.push_back(it.second);
    }

    for(auto clone : objects) {
        uint32_t originID = clone->clonedFrom();

        auto it = temp.begin();
        while(it != temp.end()) {
            const Object *origin = *it;
            if(clone->clonedFrom() == 0 || !contains(originID) || origin->uuid() == originID) {
                it = temp.erase(it);
                break;
            }
            ++it;
        }
    }

    return temp;
}
/*!
    \internal
*/
void Prefab::loadUserData(const VariantMap &data) {
    Resource::loadUserData(data);

    auto it = data.find(gActor);
    if(it != data.end()) {
        uint32_t uuid = uint32_t((*it).second.toInt());
        Object *object = Engine::findObject(uuid, Engine::findRoot(this));
        setActor(dynamic_cast<Actor *>(object));
    }
}
/*!
    \internal
*/
VariantMap Prefab::saveUserData() const {
    VariantMap result(Resource::saveUserData());

    if(m_actor) {
        result[gActor] = int(m_actor->uuid());
    }

    return result;
}
/*!
    \internal
*/
void Prefab::makeCache(Object *object) {
    if(object->clonedFrom() == 0) {
        m_dictionary[object->uuid()] = object;
    }

    for(const auto &it : object->getChildren()) {
        makeCache(it);
    }
}
