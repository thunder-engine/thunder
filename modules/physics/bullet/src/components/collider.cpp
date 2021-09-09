#include "components/collider.h"

/// \todo Temporary
#include <components/transform.h>
#include <components/actor.h>
#include <log.h>

Collider::Collider() :
        m_pCollisionShape(nullptr),
        m_pCollisionObject(nullptr),
        m_pWorld(nullptr) {

}

Collider::~Collider() {
    destroyShape();
    delete m_pCollisionObject;
}

void Collider::update() {

}

btCollisionShape *Collider::shape() {
    return m_pCollisionShape;
}

btDynamicsWorld *Collider::world() const {
    return m_pWorld;
}

void Collider::setWorld(btDynamicsWorld *world) {
    m_pWorld = world;
    if(m_pWorld) {
        createCollider();
    }
}

void Collider::createCollider() {

}

void Collider::destroyShape() {
    delete m_pCollisionShape;
    m_pCollisionShape = nullptr;
}

void Collider::dirtyContacts() {
    for(auto &it : m_Collisions) {
        it.second = true;
    }
}

void Collider::cleanContacts() {
    auto it = m_Collisions.begin();
    while(it != m_Collisions.end()) {
        if(it->second == true) {
            emitSignal(_SIGNAL(exited()));
            it = m_Collisions.erase(it);
            m_pCollisionObject->activate(true);
        } else {
            ++it;
        }
    }
}

void Collider::setContact(Collider *other) {
    bool result = true;
    for(auto &it : m_Collisions) {
        if(it.first == other->uuid()) {
            emitSignal(_SIGNAL(stay()));
            it.second = false;
            result = false;
            break;
        }
    }
    if(result) {
        emitSignal(_SIGNAL(entered()));
        m_Collisions[other->uuid()] = false;
    }
}
