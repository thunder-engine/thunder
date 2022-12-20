#include <resources/prefab.h>

#include <components/actor.h>

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
