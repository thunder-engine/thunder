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
    \inmodule Resource
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
/*!
    \internal
*/
void Prefab::loadUserData(const VariantMap &data) {
    delete p_ptr->m_pActor;
    p_ptr->m_pActor = nullptr;

    auto it = data.find(DATA);
    if(it != data.end()) {
        p_ptr->m_pActor = static_cast<Actor *>(Engine::toObject((*it).second));
        p_ptr->m_pActor->setParent(this);
    }

    setState(Ready);
}
/*!
    \internal
*/
VariantMap Prefab::saveUserData() const {
    VariantMap result;
    if(p_ptr->m_pActor) {
        result[DATA] = Engine::toVariant(p_ptr->m_pActor);
    }
    return result;
}
