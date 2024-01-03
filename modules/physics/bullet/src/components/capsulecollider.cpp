#include "components/capsulecollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <gizmos.h>

#include <btBulletDynamicsCommon.h>

/*!
    \class CapsuleCollider
    \brief The CapsuleCollider class represents a capsule-shaped collision volume attached to an actor.
    \inmodule Engine

    The CapsuleCollider class provides methods to manipulate the height of the capsule collider and obtain its associated collision shape.
    This class is designed for use in 3D physics simulations and game development.
*/

CapsuleCollider::CapsuleCollider() :
        m_height(1.0f) {

}
/*!
    Returns the height of the capsule collider.
*/
float CapsuleCollider::height() const {
    return m_height;
}
/*!
    Sets the \a height of the capsule collider.
*/
void CapsuleCollider::setHeight(float height) {
    m_height = height;
}
/*!
    \internal
    Returns the Bullet Physics collision shape associated with the capsule collider.
    If the shape does not exist or if the height has been changed, it creates a new shape.
*/
btCollisionShape *CapsuleCollider::shape() {
    if(m_collisionShape == nullptr) {
        m_collisionShape = new btCapsuleShape(m_radius, m_height - m_radius * 2.0f);

        Vector3 p = transform()->scale();
        m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_dirty = false;
    }
    return m_collisionShape;
}
/*!
    \internal
    Draws a wireframe representation of the capsule collider in the scene for visualization purposes.
    This method is typically called when the actor is selected in the editor.
*/
void CapsuleCollider::drawGizmosSelected() {
    Transform *t = transform();
    Gizmos::drawWireCapsule(m_center, m_radius, m_height, gizmoColor(), t->worldTransform());
}
