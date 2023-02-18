#include "components/spherecollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <gizmos.h>

#include <btBulletDynamicsCommon.h>

SphereCollider::SphereCollider() :
    m_radius(0.5f) {

}

float SphereCollider::radius() const {
    return m_radius;
}

void SphereCollider::setRadius(float radius) {
    m_radius = radius;
    m_dirty = true;
}

btCollisionShape *SphereCollider::shape() {
    if(m_collisionShape == nullptr) {
        m_collisionShape = new btSphereShape(m_radius);

        Vector3 p = transform()->scale();
        m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_dirty = false;
    }
    return m_collisionShape;
}

void SphereCollider::drawGizmosSelected() {
    Transform *t = transform();
    Gizmos::drawWireSphere(m_center, m_radius, gizmoColor(), t->worldTransform());
}
