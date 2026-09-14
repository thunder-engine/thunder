/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "collider.h"

class VolumeCollider;
class PhysicMaterial;
class MotionState;
class Joint;

class BULLET_EXPORT RigidBody : public Collider {
    A_OBJECT(RigidBody, Collider, Components/Physics)

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
