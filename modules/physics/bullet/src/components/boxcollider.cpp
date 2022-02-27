#include "components/boxcollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <btBulletDynamicsCommon.h>

BoxCollider::BoxCollider() :
        m_size(Vector3(0.5f)) {

}

const Vector3 &BoxCollider::size() const {
    return m_size;
}

void BoxCollider::setSize(const Vector3 &size) {
    m_size = size;
    m_dirty = true;
}

btCollisionShape *BoxCollider::shape() {
    if(m_collisionShape == nullptr) {
        m_collisionShape = new btBoxShape(btVector3(m_size.x, m_size.y, m_size.z));

        Transform *t = actor()->transform();

        Vector3 p = t->scale();
        m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_dirty = false;
    }
    return m_collisionShape;
}

#ifdef SHARED_DEFINE
#include <viewport/handles.h>

bool BoxCollider::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        Transform *t = actor()->transform();
        Handles::drawBox(t->worldPosition() + t->worldQuaternion() * m_center, t->worldRotation(), t->worldScale() * m_size * 2.0f);
    }
    return false;
}
#endif
