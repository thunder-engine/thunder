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
    delete m_pCollisionShape;
    delete m_pCollisionObject;
}

void Collider::update() {
    if(m_pCollisionObject) {
        Transform *t = actor()->transform();

        const Quaternion &q = t->quaternion();
        Vector3 p = t->position();

        m_pCollisionObject->setWorldTransform(btTransform(btQuaternion(q.x, q.y, q.z, q.w), btVector3(p.x, p.y, p.z)));
    }
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
