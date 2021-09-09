#include <resources/prefab.h>

#include <components/actor.h>

#define DATA "Data"

class PrefabPrivate {
public:
    PrefabPrivate() :
            m_pActor(nullptr) {

    }

    Actor *m_pActor;
};

/*!
    \class Prefab
    \brief A small piece of objects hierarchy which can be placed on the scene.
    \inmodule Resources
*/

Prefab::Prefab() :
        p_ptr(new PrefabPrivate) {

}

Prefab::~Prefab() {
    delete p_ptr;
}
/*!
    Returns prototype Actor which will should be instanced
*/
Actor *Prefab::actor() const {
    if(p_ptr->m_pActor == nullptr) {
        auto &children = getChildren();
        if(!children.empty()) {
            p_ptr->m_pActor = dynamic_cast<Actor *>(children.front());
        }
    }
    return p_ptr->m_pActor;
}
/*!
    \internal
*/
void Prefab::setActor(Actor *actor) {
    p_ptr->m_pActor = actor;
    if(p_ptr->m_pActor) {
        p_ptr->m_pActor->setParent(this);
    }
}
