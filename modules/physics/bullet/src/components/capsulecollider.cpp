#include "components/capsulecollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <btBulletDynamicsCommon.h>

CapsuleCollider::CapsuleCollider() :
        m_Height(1.0f) {

}

float CapsuleCollider::height() const {
    return m_Height;
}

void CapsuleCollider::setHeight(float height) {
    m_Height = height;
}

btCollisionShape *CapsuleCollider::shape() {
    if(m_pCollisionShape == nullptr) {
        m_pCollisionShape = new btCapsuleShape(m_Radius, m_Height);

        Transform *t = actor()->transform();

        Vector3 p = t->scale();
        m_pCollisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));
    }
    return m_pCollisionShape;
}

#ifdef NEXT_SHARED
#include <handles/handles.h>

bool CapsuleCollider::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        Transform *t = actor()->transform();
        Handles::drawCapsule(t->worldPosition(), t->worldRotation(), m_Radius, m_Height);
    }
    return false;
}
#endif
