#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "collider.h"

class VolumeCollider;
class PhysicMaterial;
class MotionState;
class Joint;

class BULLET_EXPORT RigidBody : public Collider {
    A_REGISTER(RigidBody, Collider, Components/Physics)

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

    int lockPosition() const;
    void setLockPosition(int flags);

    int lockRotation() const;
    void setLockRotation(int flags);

    void applyForce(const Vector3 &force, const Vector3 &point);
    void applyImpulse(const Vector3 &impulse, const Vector3 &point);

protected:
    void update() override;

    void createCollider() override;

    void setEnabled(bool enable) override;

    void updateCollider(bool updated);

    PhysicMaterial *material() const;

protected:
    std::list<VolumeCollider *> m_colliders;
    std::list<Joint *> m_joints;

    MotionState *m_state;

    float m_mass;

    int32_t m_lockPosition;
    int32_t m_lockRotation;

    bool m_kinematic;

};
typedef RigidBody* RigidBodyPtr;

#endif // RIGIDBODY_H
