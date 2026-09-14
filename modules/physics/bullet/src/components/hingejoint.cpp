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
#include "components/hingejoint.h"

#include <btBulletDynamicsCommon.h>

#include <components/transform.h>

#include <gizmos.h>

/*!
    \class HingeJoint
    \brief The HingeJoint component connects two rigid bodies around one axis.
    \inmodule Components

    A HingeJoint allows angular movement around its configured axis while
    constraining the connected bodies at their anchor positions.
*/

HingeJoint::HingeJoint() :
        m_axis(Vector3(1.0f, 0.0f, 0.0f)) {

}

/*!
    Returns the hinge rotation axis.
*/
Vector3 HingeJoint::axis() const {
    return m_axis;
}

/*!
    Sets the hinge rotation \a axis.
*/
void HingeJoint::setAxis(const Vector3 &axis) {
    m_axis = axis;

    updateParams();
}

void HingeJoint::createConstraint() {
    if(m_rigidBodyB) {
        btRigidBody *rigidBodyA = getNativeBody();
        m_constraint = new btHingeConstraint(rigidBodyA ? *rigidBodyA : btHingeConstraint::getFixedBody(), *m_rigidBodyB,
                                             btVector3(m_connectedAnchor.x, m_connectedAnchor.y, m_connectedAnchor.z),
                                             btVector3(m_anchor.x, m_anchor.y, m_anchor.z),
                                             btVector3(m_axis.x, m_axis.y, m_axis.z),
                                             btVector3(m_axis.x, m_axis.y, m_axis.z),
                                             true);
    }
}

void HingeJoint::updateParams() {

}

void HingeJoint::drawGizmosSelected() {
    Transform *t = transform();
    Matrix4 m(t->worldPosition(), t->worldQuaternion().toMatrix(), Vector3(1.0f));
    Gizmos::drawSolidBox(m_anchor, 0.1f, gizmoColor(), &m);

    if(m_rigidBodyA) {
        t = m_rigidBodyA->transform();
        m = Matrix4(t->worldPosition(), t->worldQuaternion().toMatrix(), Vector3(1.0f));
    } else {
        m.identity();
    }

    Gizmos::drawSolidBox(m_connectedAnchor, 0.1f, gizmoColor(), &m);
}
