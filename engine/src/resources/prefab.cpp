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

Prefab::Prefab() :
        p_ptr(new PrefabPrivate) {

}

Prefab::~Prefab() {
    delete p_ptr;
}

Actor *Prefab::actor() const {
    return p_ptr->m_pActor;
}

void Prefab::setActor(Actor *actor) {
    p_ptr->m_pActor = actor;
    if(p_ptr->m_pActor) {
        p_ptr->m_pActor->setParent(this);
    }
}

void Prefab::loadUserData(const VariantMap &data) {
    delete p_ptr->m_pActor;

    auto it = data.find(DATA);
    if(it != data.end()) {
        p_ptr->m_pActor = static_cast<Actor *>(Engine::toObject((*it).second));
        p_ptr->m_pActor->setParent(this);
    }

    setState(Ready);
}

VariantMap Prefab::saveUserData() const {
    VariantMap result;
    if(p_ptr->m_pActor) {
        result[DATA] = Engine::toVariant(p_ptr->m_pActor);
    }
    return result;
}
