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
#include "components/springjoint.h"

#include <btBulletDynamicsCommon.h>

#include <components/transform.h>

#include <gizmos.h>

/*!
    \class SpringJoint
    \brief The SpringJoint component connects two rigid bodies with spring forces.
    \inmodule Components

    A SpringJoint uses spring and damper parameters to control the relative
    movement of the connected rigid bodies.
*/

SpringJoint::SpringJoint() :
        m_damper(0.2f),
        m_spring(10.0f) {

}

/*!
    Returns the damping coefficient of the spring joint.
*/
float SpringJoint::damper() const {
    return m_damper;
}

/*!
    Sets the \a damper coefficient of the spring joint.
*/
void SpringJoint::setDamper(float damper) {
    m_damper = damper;

    updateParams();
}

/*!
    Returns the spring stiffness of the joint.
*/
float SpringJoint::spring() const {
    return m_spring;
}

/*!
    Sets the \a spring stiffness of the joint.
*/
void SpringJoint::setSpring(float spring) {
    m_spring = spring;

    updateParams();
}

void SpringJoint::createConstraint() {
    if(m_rigidBodyB) {
        btTransform frameInA = btTransform::getIdentity();
        frameInA.setOrigin(btVector3(m_connectedAnchor.x, m_connectedAnchor.y, m_connectedAnchor.z));

        btTransform frameInB = btTransform::getIdentity();
        frameInB.setOrigin(btVector3(m_anchor.x, m_anchor.y, m_anchor.z));

        btRigidBody *rigidBodyA = getNativeBody();
        m_constraint = new btGeneric6DofSpring2Constraint(rigidBodyA ? *rigidBodyA : btGeneric6DofSpring2Constraint::getFixedBody(),
                                                          *m_rigidBodyB,
                                                          frameInA,
                                                          frameInB);

        updateParams();
    }
}

void SpringJoint::updateParams() {
    if(m_constraint) {
        btGeneric6DofSpring2Constraint *spring = static_cast<btGeneric6DofSpring2Constraint *>(m_constraint);

        for(int i = 0; i < 3; i++) {
            spring->setLimit(i, 1, -1);
            spring->enableSpring(i, true);
            spring->setDamping(i, m_damper);
            spring->setStiffness(i, m_spring);
            spring->setEquilibriumPoint(i, 0);
        }
    }
}

void SpringJoint::drawGizmosSelected() {
    Vector4 color(gizmoColor());

    Matrix4 mA;
    if(m_rigidBodyA) {
        mA = m_rigidBodyA->transform()->worldTransform();
    }

    Matrix4 mB(transform()->worldTransform());

    Gizmos::drawSolidBox(m_connectedAnchor, 0.1f, color, &mA);
    Gizmos::drawSolidBox(m_anchor, 0.1f, color, &mB);

    Gizmos::drawLines({Vector3(mA * m_connectedAnchor), Vector3(mB * m_anchor)}, {0, 1}, color);

}
