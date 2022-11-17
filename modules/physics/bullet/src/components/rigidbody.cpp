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

class RigidBodyPrivate : public btMotionState {
public:
    explicit RigidBodyPrivate(RigidBody *ptr) :
        p_ptr(ptr),
        m_Mass(1.0f),
        m_LockPosition(0),
        m_LockRotation(0),
        m_Kinematic(false) {

    }

    void getWorldTransform(btTransform &worldTrans) const override {
        Actor *a = p_ptr->actor();
        if(a) {
            Transform *t = a->transform();
            const Quaternion &q = t->worldQuaternion();
            Vector3 p = t->worldPosition();
            worldTrans.setRotation(btQuaternion(q.x, q.y, q.z, q.w));
            worldTrans.setOrigin(btVector3(p.x, p.y, p.z));
        }
    }

    void setWorldTransform(const btTransform &worldTrans) override {
        Actor *a = p_ptr->actor();
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

    RigidBody *p_ptr;

    float m_Mass;

    list<VolumeCollider *> m_Colliders;

    int32_t m_LockPosition;
    int32_t m_LockRotation;

    bool m_Kinematic;
};

RigidBody::RigidBody() :
        p_ptr(new RigidBodyPrivate(this)) {

    m_collisionShape = new btCompoundShape;
}

RigidBody::~RigidBody() {
    if(m_world) {
        m_world->removeRigidBody(static_cast<btRigidBody *>(m_collisionObject));
    }
}

void RigidBody::update() {
    updateCollider(false);

    if(m_collisionObject && p_ptr->m_Kinematic) {
        Transform *t = actor()->transform();

        Quaternion q = t->worldQuaternion();
        Vector3 p = t->worldPosition();

        static_cast<btRigidBody *>(m_collisionObject)->setWorldTransform(btTransform(btQuaternion(q.x, q.y, q.z, q.w),
                                                                                      btVector3(p.x, p.y, p.z)));
    }
}

float RigidBody::mass() const {
    return p_ptr->m_Mass;
}

void RigidBody::setMass(float mass) {
    p_ptr->m_Mass = mass;
    if(m_collisionObject) {
        btVector3 localInertia(0, 0, 0);
        if(!p_ptr->m_Kinematic) {
            m_collisionShape->calculateLocalInertia(p_ptr->m_Mass, localInertia);
        }
        static_cast<btRigidBody *>(m_collisionObject)->setMassProps(p_ptr->m_Mass, localInertia);
    }
}

bool RigidBody::kinematic() const {
    return p_ptr->m_Kinematic;
}

void RigidBody::setKinematic(bool kinematic) {
    p_ptr->m_Kinematic = kinematic;
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
    return p_ptr->m_LockPosition;
}

void RigidBody::setLockPosition(int flags) {
    p_ptr->m_LockPosition = flags;
    if(m_collisionObject) {
        btRigidBody *body = static_cast<btRigidBody *>(m_collisionObject);
        body->setLinearFactor(btVector3((p_ptr->m_LockPosition & AXIS_X) ? 0.0 : 1.0,
                                        (p_ptr->m_LockPosition & AXIS_Y) ? 0.0 : 1.0,
                                        (p_ptr->m_LockPosition & AXIS_Z) ? 0.0 : 1.0));
    }
}

int RigidBody::lockRotation() const {
    return p_ptr->m_LockRotation;
}

void RigidBody::setLockRotation(int flags) {
    p_ptr->m_LockRotation = flags;
    if(m_collisionObject) {
        btRigidBody *body = static_cast<btRigidBody *>(m_collisionObject);
        body->setAngularFactor(btVector3((p_ptr->m_LockRotation & AXIS_X) ? 0.0 : 1.0,
                                         (p_ptr->m_LockRotation & AXIS_Y) ? 0.0 : 1.0,
                                         (p_ptr->m_LockRotation & AXIS_Z) ? 0.0 : 1.0));
    }
}

void RigidBody::createCollider() {
    updateCollider(true);

    btRigidBody *body = new btRigidBody(p_ptr->m_Mass, p_ptr, m_collisionShape);
    m_collisionObject = body;

    body->setUserPointer(this);

    setLockPosition(p_ptr->m_LockPosition);
    setLockRotation(p_ptr->m_LockRotation);

    float mass = p_ptr->m_Mass;
    PhysicMaterial *mat = material();
    if(mat) {
        body->setFriction(mat->friction());
        body->setRestitution(mat->restitution());
        mass *= mat->density();
    }
    setMass(mass);

    if(m_collisionObject && m_world) {
        m_world->addRigidBody(static_cast<btRigidBody *>(m_collisionObject));
    }
}

void RigidBody::setEnabled(bool enable) {
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

    for(auto it : p_ptr->m_Colliders) {
        if(it->isDirty()) {
            btCompoundShape *compound = static_cast<btCompoundShape *>(m_collisionShape);
            compound->removeChildShape(it->shape());
            updated = true;
        }
    }

   if(updated) {
        p_ptr->m_Colliders = actor()->findChildren<VolumeCollider *>(true);

        for(auto &it : p_ptr->m_Colliders) {
            if(!it->trigger()) {
                btTransform transform;
                transform.setIdentity();

                Transform *t = it->actor()->transform();

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
    if(!p_ptr->m_Colliders.empty()) {
        return p_ptr->m_Colliders.front()->material();
    }
    return nullptr;
}
