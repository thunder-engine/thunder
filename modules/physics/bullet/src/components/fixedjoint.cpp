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
#include "components/fixedjoint.h"

#include <btBulletDynamicsCommon.h>

#include <components/transform.h>

#include <gizmos.h>

FixedJoint::FixedJoint() {

}

void FixedJoint::createConstraint() {
    if(m_rigidBodyB) {
        btRigidBody *rigidBodyA = getNativeBody();

        btTransform frameInA = btTransform::getIdentity();
        frameInA.setOrigin(btVector3(m_connectedAnchor.x, m_connectedAnchor.y, m_connectedAnchor.z));

        btTransform frameInB = btTransform::getIdentity();
        frameInB.setOrigin(btVector3(m_anchor.x, m_anchor.y, m_anchor.z));

        m_constraint = new btFixedConstraint(rigidBodyA ? *rigidBodyA : btHingeConstraint::getFixedBody(), *m_rigidBodyB,
                                             frameInA, frameInB);
    }
}
