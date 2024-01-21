#include "components/springjoint.h"

#include <btBulletDynamicsCommon.h>

#include <components/transform.h>

#include <gizmos.h>

SpringJoint::SpringJoint() :
        m_damper(0.2f),
        m_spring(10.0f) {

}

float SpringJoint::damper() const {
    return m_damper;
}

void SpringJoint::setDamper(float damper) {
    m_damper = damper;

    updateParams();
}

float SpringJoint::spring() const {
    return m_spring;
}

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

    Gizmos::drawBox(m_connectedAnchor, 0.1f, color, mA);
    Gizmos::drawBox(m_anchor, 0.1f, color, mB);

    Gizmos::drawLines({Vector3(mA * m_connectedAnchor), Vector3(mB * m_anchor)}, {0, 1}, color, Matrix4());

}
