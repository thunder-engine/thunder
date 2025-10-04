#include "components/joint.h"

#include <btBulletDynamicsCommon.h>
#include <components/transform.h>
#include <components/actor.h>

#include "rigidbody.h"

Joint::Joint() :
        m_constraint(nullptr),
        m_world(nullptr),
        m_rigidBodyB(nullptr),
        m_rigidBodyA(nullptr),
        m_anchor(Vector3(0.0, 0.5f, 0.0f)),
        m_breakForse(std::numeric_limits<float>::infinity()),
        m_autoConfigureConnectedAnchor(true) {

}

Joint::~Joint() {
    destroyConstraint();
}

Vector3 Joint::anchor() const {
    return m_anchor;
}

void Joint::setAnchor(const Vector3 anchor) {
    m_anchor = anchor;

    updateAnchors();
}

Vector3 Joint::connectedAnchor() const {
    return m_connectedAnchor;
}

void Joint::setConnectedAnchor(Vector3 anchor) {
    m_connectedAnchor = anchor;
}

RigidBody *Joint::connectedBody() const {
    return m_rigidBodyA;
}

void Joint::setConnectedBody(RigidBody *body) {
    m_rigidBodyA = body;

    updateAnchors();
}

bool Joint::autoConfigureConnectedAnchor() const {
    return m_autoConfigureConnectedAnchor;
}

void Joint::setAutoConfigureConnectedAnchor(bool anchor) {
    m_autoConfigureConnectedAnchor = anchor;

    updateAnchors();

    if(!m_autoConfigureConnectedAnchor) {
        m_connectedAnchor = Vector3();
    }
}

void Joint::setBulletWorld(btDynamicsWorld *world) {
    destroyConstraint();

    m_world = world;
    if(m_world) {
        createConstraint();

        if(m_constraint) {
            m_constraint->setBreakingImpulseThreshold(m_breakForse);

            m_world->addConstraint(m_constraint);
        }
    }
}

void Joint::createConstraint() {
    if(m_rigidBodyB) {
        btRigidBody *rigidBodyA = getNativeBody();

        if(rigidBodyA) {
            m_constraint = new btPoint2PointConstraint(*rigidBodyA, *m_rigidBodyB,
                                                       btVector3(m_anchor.x, m_anchor.y, m_anchor.z),
                                                       btVector3(m_connectedAnchor.x, m_connectedAnchor.y, m_connectedAnchor.z));
        } else {
            m_constraint = new btPoint2PointConstraint(*m_rigidBodyB,
                                                       btVector3(m_anchor.x, m_anchor.y, m_anchor.z));
        }
    }
}

void Joint::destroyConstraint() {
    if(m_world && m_constraint) {
        m_world->removeConstraint(m_constraint);

        delete m_constraint;
        m_constraint = nullptr;
    }
}

btRigidBody *Joint::getNativeBody() {
    if(m_rigidBodyA) {
        return dynamic_cast<btRigidBody *>(m_rigidBodyA->collisionObject());
    }
    return nullptr;
}

Vector4 Joint::gizmoColor() const {
    return  Vector4(1.0f, 0.5f, 0.0f, 1.0f);
}

void Joint::updateAnchors() {
    if(m_autoConfigureConnectedAnchor) {
        if(m_rigidBodyB == nullptr) {
            m_connectedAnchor = transform()->worldPosition() + m_anchor;
        }
    }
}
/*!
    \internal
*/
void Joint::setRigidBodyA(btRigidBody *body) {
    m_rigidBodyB = body;
}
/*!
    \internal
*/
void Joint::composeComponent() {
    Component::composeComponent();

    Actor *a = actor();
    if(a) {
        RigidBody *rigidBody = a->findChild<RigidBody *>();

        if(rigidBody == nullptr) {
            rigidBody = Engine::objectCreate<RigidBody>(std::string(), a);
            rigidBody->composeComponent();
        }

        updateAnchors();
    }
}
