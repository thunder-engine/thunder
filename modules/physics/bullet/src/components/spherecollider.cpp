/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "components/spherecollider.h"

#include <components/actor.h>
#include <components/transform.h>

#include <gizmos.h>

#include <btBulletDynamicsCommon.h>

/*!
    \class SphereCollider
    \brief The SphereCollider class represents a spherical collider component, defining a spherical shape for collision detection.
    \inmodule Bullet

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
    Gizmos::drawWireSphere(m_center, m_radius, gizmoColor(), &t->worldTransform());
}
