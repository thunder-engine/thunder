#include "components/capsulecollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <gizmos.h>

#include <btBulletDynamicsCommon.h>

CapsuleCollider::CapsuleCollider() :
        m_height(1.0f) {

}

float CapsuleCollider::height() const {
    return m_height;
}

void CapsuleCollider::setHeight(float height) {
    m_height = height;
}

btCollisionShape *CapsuleCollider::shape() {
    if(m_collisionShape == nullptr) {
        m_collisionShape = new btCapsuleShape(m_radius, m_height - m_radius * 2.0f);

        Vector3 p = transform()->scale();
        m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_dirty = false;
    }
    return m_collisionShape;
}

void CapsuleCollider::drawGizmosSelected() {
    Transform *t = transform();
    Gizmos::drawWireCapsule(m_center, m_radius, m_height, gizmoColor(), t->worldTransform());
}
