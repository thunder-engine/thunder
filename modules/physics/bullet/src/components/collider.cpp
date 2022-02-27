#include "components/collider.h"

#include <btBulletDynamicsCommon.h>

Collider::Collider() :
        m_collisionShape(nullptr),
        m_collisionObject(nullptr),
        m_world(nullptr) {

}

Collider::~Collider() {
    destroyShape();
    destroyCollider();
}

void Collider::update() {

}

btDynamicsWorld *Collider::world() const {
    return m_world;
}

void Collider::setWorld(btDynamicsWorld *world) {
    m_world = world;
    if(m_world) {
        createCollider();
    }
}

void Collider::createCollider() {

}

btCollisionShape *Collider::shape() {
    return m_collisionShape;
}

void Collider::destroyShape() {
    delete m_collisionShape;
    m_collisionShape = nullptr;
}

void Collider::destroyCollider() {
    delete m_collisionObject;
    m_collisionObject = nullptr;
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
            if(m_collisionObject) {
                m_collisionObject->activate(true);
            }
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
