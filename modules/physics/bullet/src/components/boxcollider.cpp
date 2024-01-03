#include "components/boxcollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <gizmos.h>

#include <btBulletDynamicsCommon.h>

/*!
    \class BoxCollider
    \brief The BoxCollider component represents a box-shaped collision volume attached to an actor.
    \inmodule Engine

    The BoxCollider class provides methods to manipulate the size of the box collider and obtain its associated collision shape.
    This class is designed for use in 3D physics simulations and game development.
*/

BoxCollider::BoxCollider() :
        m_size(Vector3(0.5f)) {

}
/*!
    Returns the size of the box collider.
*/
const Vector3 &BoxCollider::size() const {
    return m_size;
}
/*!
    Sets the size of the box collider.
*/
void BoxCollider::setSize(const Vector3 size) {
    m_size = size;
    m_dirty = true;
}
/*!
    \internal
    Returns the Bullet Physics collision shape associated with the box collider.
    If the shape does not exist or if the size has been changed, it creates a new shape.
*/
btCollisionShape *BoxCollider::shape() {
    if(m_collisionShape == nullptr) {
        m_collisionShape = new btBoxShape(btVector3(m_size.x, m_size.y, m_size.z));

        Vector3 p = transform()->scale();
        m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_dirty = false;
    }
    return m_collisionShape;
}
/*!
    \internal
    Draws wireframe representation of the box collider in the scene for visualization purposes.
    This method is typically called when the actor is selected in the editor.
*/
void BoxCollider::drawGizmosSelected() {
    Transform *t = transform();
    Gizmos::drawWireBox(m_center, m_size * 2.0f, gizmoColor(), t->worldTransform());
}
