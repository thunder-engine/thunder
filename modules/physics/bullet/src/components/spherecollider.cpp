#include "components/spherecollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <gizmos.h>

#include <btBulletDynamicsCommon.h>

/*!
    \class SphereCollider
    \brief The SphereCollider class represents a spherical collider component, defining a spherical shape for collision detection.
    \inmodule Components

    The SphereCollider class provides functionality to define a sphere collider with a specific radius.
    It supports retrieving and setting the radius.
    The sphere collider can be integrated with other components in a game or simulation to enable accurate collision detection.
*/

SphereCollider::SphereCollider() :
        m_radius(0.5f) {

}
/*!
    Returns the radius of the sphere collider.
*/
float SphereCollider::radius() const {
    return m_radius;
}
/*!
    Sets the \a radius of the sphere collider.
*/
void SphereCollider::setRadius(float radius) {
    m_radius = radius;
    m_dirty = true;
}
/*!
    \internal
    Returns the Bullet Physics collision shape representing the sphere collider.
    If the shape does not exist, it is created.
*/
btCollisionShape *SphereCollider::shape() {
    if(m_collisionShape == nullptr) {
        m_collisionShape = new btSphereShape(m_radius);

        Vector3 p = transform()->scale();
        m_collisionShape->setLocalScaling(btVector3(p.x, p.y, p.z));

        m_dirty = false;
    }
    return m_collisionShape;
}
/*!
    \internal
    Draws wireframe representation of the sphere collider using Gizmos.
*/
void SphereCollider::drawGizmosSelected() {
    Transform *t = transform();
    Gizmos::drawWireSphere(m_center, m_radius, gizmoColor(), t->worldTransform());
}
