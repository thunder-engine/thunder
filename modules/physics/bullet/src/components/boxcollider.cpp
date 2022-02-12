#include "components/boxcollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <btBulletDynamicsCommon.h>

BoxCollider::BoxCollider() :
        m_Size(Vector3(0.5f)) {

}

const Vector3 &BoxCollider::size() const {
    return m_Size;
}

void BoxCollider::setSize(const Vector3 &size) {
    m_Size = size;
    m_Dirty = true;
}

btCollisionShape *BoxCollider::shape() {
    if(m_pCollisionShape == nullptr) {
        m_pCollisionShape = new btBoxShape(btVector3(m_Size.x, m_Size.y, m_Size.z));

        Transform *t = actor()->transform();

        Vector3 p = t->scale();
        m_pCollisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_Dirty = false;
    }
    return m_pCollisionShape;
}

#ifdef SHARED_DEFINE
#include <viewport/handles.h>

bool BoxCollider::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        Transform *t = actor()->transform();
        Handles::drawBox(t->worldPosition() + t->worldQuaternion() * m_Center, t->worldRotation(), t->worldScale() * m_Size * 2.0f);
    }
    return false;
}
#endif
