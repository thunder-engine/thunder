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

    createCollider();
}

void Collider::createCollider() {

}

void Collider::destroyShape() {
    delete m_pCollisionShape;
    m_pCollisionShape = nullptr;
}
