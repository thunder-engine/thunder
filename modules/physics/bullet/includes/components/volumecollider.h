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
#ifndef VOLUMECOLLIDER_H
#define VOLUMECOLLIDER_H

#include <collider.h>

#include <physicmaterial.h>

class PhysicMaterial;

class BULLET_EXPORT VolumeCollider : public Collider {
    A_OBJECT(VolumeCollider, Collider, General)

    A_PROPERTIES(
        A_PROPERTY(bool, trigger, VolumeCollider::trigger, VolumeCollider::setTrigger),
        A_PROPERTYEX(PhysicMaterial *, material, VolumeCollider::material, VolumeCollider::setMaterial, "editor=Asset"),
        A_PROPERTY(Vector3, center, VolumeCollider::center, VolumeCollider::setCenter)
    )

public:
     VolumeCollider();
    ~VolumeCollider() override;

    bool trigger() const;
    void setTrigger(bool trigger);

    PhysicMaterial *material() const;
    void setMaterial(PhysicMaterial *material);

    const Vector3 &center() const;
    void setCenter(const Vector3 center);

    void retrieveContact(const Collider *collider) const;

    bool isDirty() const;

protected:
    void createCollider() override;

    void update() override;

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

protected:
    typedef std::unordered_map<uint32_t, bool> CollisionMap;

    CollisionMap m_collisions;

    Vector3 m_center;

    PhysicMaterial *m_material;

    bool m_dirty;

    bool m_trigger;

};
typedef VolumeCollider* VolumeColliderPtr;

#endif // VOLUMECOLLIDER_H
