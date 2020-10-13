#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "collider.h"

enum Axises {
    AXIS_X = (1<<0),
    AXIS_Y = (1<<1),
    AXIS_Z = (1<<2)
};

class RigidBody : public Collider, public btMotionState {
    A_REGISTER(RigidBody, Collider, Components)

    A_PROPERTIES(
        A_PROPERTY(float, mass, RigidBody::mass, RigidBody::setMass),
        A_PROPERTYEX(int, lockPosition, RigidBody::lockPosition, RigidBody::setLockPosition, "editor=Axises"),
        A_PROPERTYEX(int, lockRotation, RigidBody::lockRotation, RigidBody::setLockRotation, "editor=Axises")
    )
    A_NOMETHODS()

public:
    RigidBody ();
    ~RigidBody () override;

    float mass () const;
    void setMass (float mass);

    void applyForce(const Vector3 &force, const Vector3 &point = Vector3());
    void applyImpulse(const Vector3 &impulse, const Vector3 &point = Vector3());

    int lockPosition () const;
    void setLockPosition (int flags);

    int lockRotation () const;
    void setLockRotation (int flags);

protected:
    void getWorldTransform (btTransform &worldTrans) const override;
    void setWorldTransform (const btTransform &worldTrans) override;

    void createCollider() override;

protected:
    float m_Mass;

    int32_t m_LockPosition;
    int32_t m_LockRotation;
};

#endif // RIGIDBODY_H
