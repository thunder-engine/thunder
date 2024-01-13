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
    if(m_rigidBodyA) {
        btRigidBody *rigidBodyB = getNativeBody();

        Vector3 p = m_connectedAnchor - transform()->worldPosition();

        if(rigidBodyB) {
            btTransform frameInA = btTransform::getIdentity();
            btTransform frameInB = btTransform::getIdentity();

            m_constraint = new btGeneric6DofSpring2Constraint(*m_rigidBodyA, *rigidBodyB,
                                                       frameInA, frameInB);
        } else {
            btTransform frameInA = btTransform::getIdentity();
            frameInA.setOrigin(btVector3(p.x, p.y, p.z));

            m_constraint = new btGeneric6DofSpring2Constraint(*m_rigidBodyA,
                                                       frameInA);
        }

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
        }
    }
}

void SpringJoint::drawGizmosSelected() {
    Vector4 color(gizmoColor());

    Matrix4 m;

    Vector3 position(transform()->worldPosition() + m_anchor);

    Gizmos::drawBox(m_connectedAnchor, 0.1f, color, m);
    Gizmos::drawBox(position, 0.1f, color, m);

    Gizmos::drawLines({position, m_connectedAnchor}, {0, 1}, color, m);
}
