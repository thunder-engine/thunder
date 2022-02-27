#include "components/capsulecollider.h"

#include <components/actor.h>
#include <components/transform.h>

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
        m_collisionShape = new btCapsuleShape(m_radius, m_height);

        Transform *t = actor()->transform();

        Vector3 p = t->scale();
        m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_dirty = false;
    }
    return m_collisionShape;
}

#ifdef SHARED_DEFINE
#include <viewport/handles.h>

bool CapsuleCollider::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        Transform *t = actor()->transform();
        Handles::drawCapsule(t->worldPosition() + t->worldQuaternion() * m_center, t->worldRotation(), m_radius, m_height);
    }
    return false;
}
#endif
