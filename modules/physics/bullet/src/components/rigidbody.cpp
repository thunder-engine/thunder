#include "components/rigidbody.h"

#include <components/actor.h>
#include <components/transform.h>

#include "components/volumecollider.h"

#include "resources/physicmaterial.h"

RigidBody::RigidBody() :
        m_Mass(1.0f),
        m_LockPosition(0),
        m_LockRotation(0) {

}

RigidBody::~RigidBody() {
    if(m_pWorld) {
        m_pWorld->removeRigidBody(static_cast<btRigidBody *>(m_pCollisionObject));
    }
}

float RigidBody::mass() const {
    return m_Mass;
}

void RigidBody::setMass(float mass) {
    m_Mass = mass;
    if(m_pCollisionObject) {
        bool dynamic = (m_Mass != 0.0f);

        btVector3 localInertia(0, 0, 0);
        if(dynamic) {
            m_pCollisionShape->calculateLocalInertia(m_Mass, localInertia);
        }
        static_cast<btRigidBody *>(m_pCollisionObject)->setMassProps(m_Mass, localInertia);
    }
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
}

int RigidBody::lockRotation() const {
    return m_LockRotation;
}

void RigidBody::setLockRotation(int flags) {
    m_LockRotation = flags;
}

void RigidBody::getWorldTransform(btTransform &worldTrans) const {
    Actor *a = actor();
    if(a) {
        Transform *t = a->transform();
        const Quaternion &q = t->rotation();
        Vector3 p = t->position();
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

        t->setRotation(rot);

        btVector3 p = worldTrans.getOrigin();
        t->setPosition(Vector3(p.x(), p.y(), p.z()));
    }
}

void RigidBody::createCollider() {
    btCompoundShape *compound = new btCompoundShape;

    PhysicMaterial *mat = nullptr;
    for(auto &it : actor()->findChildren<VolumeCollider *>(false)) {
        if(!it->trigger()) {
            btTransform transform;
            transform.setIdentity();
            transform.setOrigin(btVector3(0, 0, 0));

            compound->addChildShape(transform, it->shape());

            if(mat == nullptr) {
                mat = it->material();
            }
        }
    }
    m_pCollisionShape = compound;

    btRigidBody *body = new btRigidBody(m_Mass, this, m_pCollisionShape);
    body->setCollisionShape(m_pCollisionShape);
    body->setUserPointer(this);
    body->setLinearFactor(btVector3((m_LockPosition & AXIS_X) ? 0.0 : 1.0,
                                    (m_LockPosition & AXIS_Y) ? 0.0 : 1.0,
                                    (m_LockPosition & AXIS_Z) ? 0.0 : 1.0));

    body->setAngularFactor(btVector3((m_LockRotation & AXIS_X) ? 0.0 : 1.0,
                                     (m_LockRotation & AXIS_Y) ? 0.0 : 1.0,
                                     (m_LockRotation & AXIS_Z) ? 0.0 : 1.0));

    if(mat) {
        body->setFriction(mat->friction());
        body->setRestitution(mat->bounciness());
    }

    m_pCollisionObject = body;

    setMass(m_Mass);

    if(m_pCollisionObject) {
        m_pWorld->addRigidBody(static_cast<btRigidBody *>(m_pCollisionObject));
    }
}
