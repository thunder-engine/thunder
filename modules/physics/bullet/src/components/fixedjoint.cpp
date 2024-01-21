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
