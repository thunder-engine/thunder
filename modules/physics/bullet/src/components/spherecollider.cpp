#include "components/spherecollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <btBulletDynamicsCommon.h>

SphereCollider::SphereCollider() :
    m_Radius(0.5f) {

}

float SphereCollider::radius() const {
    return m_Radius;
}

void SphereCollider::setRadius(float radius) {
    m_Radius = radius;
}

btCollisionShape *SphereCollider::shape() {
    if(m_pCollisionShape == nullptr) {
        m_pCollisionShape = new btSphereShape(m_Radius);

        Transform *t = actor()->transform();

        Vector3 p = t->scale();
        m_pCollisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_Dirty = false;
    }
    return m_pCollisionShape;
}

#ifdef NEXT_SHARED
#include <editor/handles.h>

bool SphereCollider::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        Transform *t = actor()->transform();
        Handles::drawSphere(t->worldPosition(), t->worldRotation(), m_Radius);
    }
    return false;
}
#endif
