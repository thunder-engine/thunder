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
    m_actor = actor;
    if(m_actor) {
        m_actor->setParent(this);
    }
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
