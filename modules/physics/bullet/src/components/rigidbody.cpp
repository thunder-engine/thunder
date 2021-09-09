#include "components/rigidbody.h"

#include <components/actor.h>
#include <components/transform.h>

#include <log.h>

#include "components/volumecollider.h"

#include "resources/physicmaterial.h"

RigidBody::RigidBody() :
        m_Mass(1.0f),
        m_LockPosition(0),
        m_LockRotation(0),
        m_Kinematic(false) {

    m_pCollisionShape = new btCompoundShape;
}

RigidBody::~RigidBody() {
    if(m_pWorld) {
        m_pWorld->removeRigidBody(static_cast<btRigidBody *>(m_pCollisionObject));
    }
}

void RigidBody::update() {
    for(auto it : m_Colliders) {
        if(it->isDirty()) {
            btCompoundShape *compound = static_cast<btCompoundShape *>(m_pCollisionShape);
            compound->removeChildShape(it->shape());

            if(!it->trigger()) {
                it->destroyShape();

                btTransform transform;
                transform.setIdentity();

                const Vector3 &center = it->center();
                transform.setOrigin(btVector3(center.x, center.y, center.z));

                compound->addChildShape(transform, it->shape());

                btRigidBody *body = static_cast<btRigidBody *>(m_pCollisionObject);
                body->setCollisionShape(m_pCollisionShape);
            }
        }
    }

    if(m_pCollisionObject && m_Kinematic) {
        Transform *t = actor()->transform();

        Quaternion &q = t->worldQuaternion();
        Vector3 &p = t->worldPosition();

        static_cast<btRigidBody *>(m_pCollisionObject)->setWorldTransform(btTransform(btQuaternion(q.x, q.y, q.z, q.w),
                                                                                      btVector3(p.x, p.y, p.z)));
    }
}

float RigidBody::mass() const {
    return m_Mass;
}

void RigidBody::setMass(float mass) {
    m_Mass = mass;
    if(m_pCollisionObject) {
        btVector3 localInertia(0, 0, 0);
        if(!m_Kinematic) {
            m_pCollisionShape->calculateLocalInertia(m_Mass, localInertia);
        }
        static_cast<btRigidBody *>(m_pCollisionObject)->setMassProps(m_Mass, localInertia);
    }
}

bool RigidBody::kinematic() const {
    return m_Kinematic;
}

void RigidBody::setKinematic(bool kinematic) {
    m_Kinematic = kinematic;
}

void RigidBody::applyForce(const Vector3 &force, const Vector3 &point) {
    static_cast<btRigidBody *>(m_pCollisionObject)->applyForce(btVector3(force.x, force.y, force.z),
                                                               btVector3(point.x, point.y, point.z));
}

void RigidBody::applyImpulse(const Vector3 &impulse, const Vector3 &point) {
    m_pCollisionObject->activate(true);
    static_cast<btRigidBody *>(m_pCollisionObject)->applyImpulse(btVector3(impulse.x, impulse.y, impulse.z),
                                                                 btVector3(point.x, point.y, point.z));
}

int RigidBody::lockPosition() const {
    return m_LockPosition;
}

void RigidBody::setLockPosition(int flags) {
    m_LockPosition = flags;
    if(m_pCollisionObject) {
        btRigidBody *body = static_cast<btRigidBody *>(m_pCollisionObject);
        body->setLinearFactor(btVector3((m_LockPosition & AXIS_X) ? 0.0 : 1.0,
                                        (m_LockPosition & AXIS_Y) ? 0.0 : 1.0,
                                        (m_LockPosition & AXIS_Z) ? 0.0 : 1.0));
    }
}

int RigidBody::lockRotation() const {
    return m_LockRotation;
}

void RigidBody::setLockRotation(int flags) {
    m_LockRotation = flags;
    if(m_pCollisionObject) {
        btRigidBody *body = static_cast<btRigidBody *>(m_pCollisionObject);
        body->setAngularFactor(btVector3((m_LockRotation & AXIS_X) ? 0.0 : 1.0,
                                         (m_LockRotation & AXIS_Y) ? 0.0 : 1.0,
                                         (m_LockRotation & AXIS_Z) ? 0.0 : 1.0));
    }
}

void RigidBody::getWorldTransform(btTransform &worldTrans) const {
    Actor *a = actor();
    if(a) {
        Transform *t = a->transform();
        const Quaternion &q = t->worldQuaternion();
        Vector3 p = t->worldPosition();
        worldTrans.setRotation(btQuaternion(q.x, q.y, q.z, q.w));
        worldTrans.setOrigin(btVector3(p.x, p.y, p.z));
    }
}

void RigidBody::setWorldTransform(const btTransform &worldTrans) {
    Actor *a = actor();
    if(a) {
        Transform *t = a->transform();
        btQuaternion q = worldTrans.getRotation();

        Quaternion rot;
        rot.x = q.getX();
        rot.y = q.getY();
        rot.z = q.getZ();
        rot.w = q.getW();

        t->setQuaternion(rot);

        btVector3 p = worldTrans.getOrigin();
        Vector3 position(p.x(), p.y(), p.z());

        Transform *parent = t->parentTransform();
        if(parent) {
            t->setPosition(parent->worldTransform().inverse() * position);
        } else {
            t->setPosition(position);
        }
    }
}

void RigidBody::createCollider() {
    btCompoundShape *compound = static_cast<btCompoundShape *>(m_pCollisionShape);

    m_Colliders = actor()->findChildren<VolumeCollider *>(false);

    PhysicMaterial *mat = nullptr;
    for(auto &it : m_Colliders) {
        if(!it->trigger()) {
            btTransform transform;
            transform.setIdentity();

            const Vector3 &center = it->center();
            transform.setOrigin(btVector3(center.x, center.y, center.z));

            compound->addChildShape(transform, it->shape());

            if(mat == nullptr) {
                mat = it->material();
            }
        }
    }

    btRigidBody *body = new btRigidBody(m_Mass, this, m_pCollisionShape);
    m_pCollisionObject = body;

    body->setUserPointer(this);

    setLockPosition(m_LockPosition);
    setLockRotation(m_LockRotation);

    float mass = m_Mass;
    if(mat) {
        body->setFriction(mat->friction());
        body->setRestitution(mat->restitution());
        mass *= mat->density();
    }
    setMass(mass);

    if(m_pCollisionObject && m_pWorld) {
        m_pWorld->addRigidBody(static_cast<btRigidBody *>(m_pCollisionObject));
    }
}
