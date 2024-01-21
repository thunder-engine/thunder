#include "components/hingejoint.h"

#include <btBulletDynamicsCommon.h>

#include <components/transform.h>

#include <gizmos.h>

HingeJoint::HingeJoint() :
        m_axis(Vector3(1.0f, 0.0f, 0.0f)) {

}

Vector3 HingeJoint::axis() const {
    return m_axis;
}

void HingeJoint::setAxis(Vector3 axis) {
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
    Gizmos::drawBox(m_anchor, 0.1f, gizmoColor(), Matrix4(t->worldPosition(),
                                                          t->worldQuaternion().toMatrix(),
                                                          Vector3(1.0f)));
    Matrix4 m;
    if(m_rigidBodyA) {
        t = m_rigidBodyA->transform();

        m = Matrix4(t->worldPosition(), t->worldQuaternion().toMatrix(), Vector3(1.0f));
    }

    Gizmos::drawBox(m_connectedAnchor, 0.1f, gizmoColor(), m);
}
