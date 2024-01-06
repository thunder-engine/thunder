#include <joint.h>

#include <btBulletDynamicsCommon.h>

#include "rigidbody.h"

Joint::Joint() :
        m_constraint(nullptr),
        m_world(nullptr),
        m_rigidBodyA(nullptr),
        m_rigidBodyB(nullptr),
        m_breakForse(std::numeric_limits<float>::infinity()) {

}

Joint::~Joint() {
    destroyConstraint();
}

RigidBody *Joint::connectedBody() const {
    return m_rigidBodyB;
}

void Joint::setBulletWorld(btDynamicsWorld *world) {
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
    //m_constraint = new btPoint2PointConstraint(m_rigidBodyA->)

}

void Joint::destroyConstraint() {
    if(m_world && m_constraint) {
        m_world->removeConstraint(m_constraint);

        delete m_constraint;
        m_constraint = nullptr;
    }
}
