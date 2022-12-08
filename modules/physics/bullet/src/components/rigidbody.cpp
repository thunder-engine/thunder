#include "components/rigidbody.h"

#include <components/actor.h>
#include <components/transform.h>

#include "components/volumecollider.h"

#include "resources/physicmaterial.h"

#include <btBulletDynamicsCommon.h>

enum Axises {
    AXIS_X = (1<<0),
    AXIS_Y = (1<<1),
    AXIS_Z = (1<<2)
};

class MotionState : public btMotionState {
public:
    explicit MotionState(RigidBody *body) :
        m_body(body) {

    }

    void getWorldTransform(btTransform &worldTrans) const override {
        Transform *t = m_body->transform();
        const Quaternion &q = t->worldQuaternion();
        Vector3 p = t->worldPosition();
        worldTrans.setRotation(btQuaternion(q.x, q.y, q.z, q.w));
        worldTrans.setOrigin(btVector3(p.x, p.y, p.z));
    }

    void setWorldTransform(const btTransform &worldTrans) override {
        Transform *t = m_body->transform();
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

private:
    RigidBody *m_body;

};

RigidBody::RigidBody() :
        m_state(new MotionState(this)),
        m_mass(1.0f),
        m_lockPosition(0),
        m_lockRotation(0),
        m_kinematic(false) {

    m_collisionShape = new btCompoundShape;
}

RigidBody::~RigidBody() {
    if(m_world && m_collisionObject) {
        m_world->removeRigidBody(static_cast<btRigidBody *>(m_collisionObject));
    }

    for(auto it : m_colliders) {
        it->setAttachedRigidBody(nullptr);
    }
    m_colliders.clear();

    delete m_state;
}

void RigidBody::update() {
    updateCollider(false);

    if(m_collisionObject && m_kinematic) {
        Transform *t = transform();

        Quaternion q = t->worldQuaternion();
        Vector3 p = t->worldPosition();

        static_cast<btRigidBody *>(m_collisionObject)->setWorldTransform(btTransform(btQuaternion(q.x, q.y, q.z, q.w),
                                                                                      btVector3(p.x, p.y, p.z)));
    }
}

float RigidBody::mass() const {
    return m_mass;
}

void RigidBody::setMass(float mass) {
    m_mass = mass;
    if(m_collisionObject) {
        btVector3 localInertia(0, 0, 0);
        if(!m_kinematic) {
            m_collisionShape->calculateLocalInertia(m_mass, localInertia);
        }
        static_cast<btRigidBody *>(m_collisionObject)->setMassProps(m_mass, localInertia);
    }
}

bool RigidBody::kinematic() const {
    return m_kinematic;
}

void RigidBody::setKinematic(bool kinematic) {
    m_kinematic = kinematic;
}

void RigidBody::applyForce(const Vector3 &force, const Vector3 &point) {
    static_cast<btRigidBody *>(m_collisionObject)->applyForce(btVector3(force.x, force.y, force.z),
                                                               btVector3(point.x, point.y, point.z));
}

void RigidBody::applyImpulse(const Vector3 &impulse, const Vector3 &point) {
    m_collisionObject->activate(true);
    static_cast<btRigidBody *>(m_collisionObject)->applyImpulse(btVector3(impulse.x, impulse.y, impulse.z),
                                                                 btVector3(point.x, point.y, point.z));
}

int RigidBody::lockPosition() const {
    return m_lockPosition;
}

void RigidBody::setLockPosition(int flags) {
    m_lockPosition = flags;
    if(m_collisionObject) {
        btRigidBody *body = static_cast<btRigidBody *>(m_collisionObject);
        body->setLinearFactor(btVector3((m_lockPosition & AXIS_X) ? 0.0 : 1.0,
                                        (m_lockPosition & AXIS_Y) ? 0.0 : 1.0,
                                        (m_lockPosition & AXIS_Z) ? 0.0 : 1.0));
    }
}

int RigidBody::lockRotation() const {
    return m_lockRotation;
}

void RigidBody::setLockRotation(int flags) {
    m_lockRotation = flags;
    if(m_collisionObject) {
        btRigidBody *body = static_cast<btRigidBody *>(m_collisionObject);
        body->setAngularFactor(btVector3((m_lockRotation & AXIS_X) ? 0.0 : 1.0,
                                         (m_lockRotation & AXIS_Y) ? 0.0 : 1.0,
                                         (m_lockRotation & AXIS_Z) ? 0.0 : 1.0));
    }
}

void RigidBody::createCollider() {
    updateCollider(true);

    btRigidBody *body = new btRigidBody(m_mass, m_state, m_collisionShape);
    m_collisionObject = body;

    body->setUserPointer(this);

    setLockPosition(m_lockPosition);
    setLockRotation(m_lockRotation);

    float mass = m_mass;
    PhysicMaterial *mat = material();
    if(mat) {
        body->setFriction(mat->friction());
        body->setRestitution(mat->restitution());
        mass *= mat->density();
    }
    setMass(mass);

    if(isEnabled() && m_collisionObject && m_world) {
        m_world->addRigidBody(static_cast<btRigidBody *>(m_collisionObject));
    }
}

void RigidBody::setEnabled(bool enable) {
    Collider::setEnabled(enable);
    if(m_collisionObject && m_world) {
        if(enable) {
            m_world->addRigidBody(static_cast<btRigidBody *>(m_collisionObject));
        } else {
            m_world->removeRigidBody(static_cast<btRigidBody *>(m_collisionObject));
        }
    }
}

void RigidBody::updateCollider(bool updated) {
    btCompoundShape *compound = static_cast<btCompoundShape *>(m_collisionShape);

    for(auto it : m_colliders) {
        if(it->isDirty()) {
            btCompoundShape *compound = static_cast<btCompoundShape *>(m_collisionShape);
            compound->removeChildShape(it->shape());
            updated = true;
        }
    }
    m_colliders.clear();

    if(updated) {
        m_colliders = actor()->findChildren<VolumeCollider *>(true);

        for(auto &it : m_colliders) {
            it->setAttachedRigidBody(this);

            if(!it->trigger()) {
                btTransform transform;
                transform.setIdentity();

                Transform *t = it->transform();

                Vector3 center = it->center();
                if(it->actor() != actor()) {
                    center += t->position();

                    const Quaternion &q = t->quaternion();
                    transform.setRotation(btQuaternion(q.x, q.y, q.z, q.w));
                }

                transform.setOrigin(btVector3(center.x, center.y, center.z));

                compound->addChildShape(transform, it->shape());
            }
        }
    }
}

PhysicMaterial *RigidBody::material() const {
    if(!m_colliders.empty()) {
        return m_colliders.front()->material();
    }
    return nullptr;
}
