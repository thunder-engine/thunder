#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "collider.h"

enum Axises {
    AXIS_X =(1<<0),
    AXIS_Y =(1<<1),
    AXIS_Z =(1<<2)
};

class VolumeCollider;

class RigidBody : public Collider, public btMotionState {
    A_REGISTER(RigidBody, Collider, Components)

    A_PROPERTIES(
        A_PROPERTY(float, mass, RigidBody::mass, RigidBody::setMass),
        A_PROPERTY(bool, kinematic, RigidBody::kinematic, RigidBody::setKinematic),
        A_PROPERTYEX(int, lockPosition, RigidBody::lockPosition, RigidBody::setLockPosition, "editor=Axises"),
        A_PROPERTYEX(int, lockRotation, RigidBody::lockRotation, RigidBody::setLockRotation, "editor=Axises")
    )
    A_METHODS(
        A_METHOD(void, RigidBody::applyForce),
        A_METHOD(void, RigidBody::applyImpulse)
    )

public:
    RigidBody();
    ~RigidBody() override;

    float mass() const;
    void setMass(float mass);

    bool kinematic() const;
    void setKinematic(bool kinematic);

    void applyForce(const Vector3 &force, const Vector3 &point);
    void applyImpulse(const Vector3 &impulse, const Vector3 &point);

    int lockPosition() const;
    void setLockPosition(int flags);

    int lockRotation() const;
    void setLockRotation(int flags);

protected:
    void update() override;

    void getWorldTransform(btTransform &worldTrans) const override;
    void setWorldTransform(const btTransform &worldTrans) override;

    void createCollider() override;

protected:
    float m_Mass;

    list<VolumeCollider *> m_Colliders;

    int32_t m_LockPosition;
    int32_t m_LockRotation;

    bool m_Kinematic;
};

#endif // RIGIDBODY_H
