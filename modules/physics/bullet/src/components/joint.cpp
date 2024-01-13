#include "components/joint.h"

#include <btBulletDynamicsCommon.h>
#include <components/transform.h>
#include <components/actor.h>

#include "rigidbody.h"

Joint::Joint() :
        m_constraint(nullptr),
        m_world(nullptr),
        m_rigidBodyA(nullptr),
        m_rigidBodyB(nullptr),
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
    return m_rigidBodyB;
}

void Joint::setConnectedBody(RigidBody *body) {
    m_rigidBodyB = body;

    updateAnchors();
}

bool Joint::autoConfigureConnectedAnchor() const {
    return m_autoConfigureConnectedAnchor;
}

void Joint::setAutoConfigureConnectedAnchor(bool anchor) {
    m_autoConfigureConnectedAnchor = anchor;

    updateAnchors();
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
    if(m_rigidBodyA) {
        btRigidBody *rigidBodyB = getNativeBody();

        if(rigidBodyB) {
            m_constraint = new btPoint2PointConstraint(*m_rigidBodyA, *rigidBodyB,
                                                       btVector3(m_anchor.x, m_anchor.y, m_anchor.z),
                                                       btVector3(m_connectedAnchor.x, m_connectedAnchor.y, m_connectedAnchor.z));
        } else {
            m_constraint = new btPoint2PointConstraint(*m_rigidBodyA,
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
    if(m_rigidBodyB) {
        return dynamic_cast<btRigidBody *>(m_rigidBodyB->collisionObject());
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
    } else {
        m_connectedAnchor = Vector3();
    }
}
/*!
    \internal
*/
void Joint::setRigidBodyA(btRigidBody *body) {
    m_rigidBodyA = body;
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
            rigidBody = Engine::objectCreate<RigidBody>(string(), a);
            rigidBody->composeComponent();
        }

        updateAnchors();
    }
}
