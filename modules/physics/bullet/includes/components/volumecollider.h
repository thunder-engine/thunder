#ifndef VOLUMECOLLIDER_H
#define VOLUMECOLLIDER_H

#include "collider.h"

class PhysicMaterial;

class BULLET_EXPORT VolumeCollider : public Collider {
    A_REGISTER(VolumeCollider, Collider, General)

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

    void retrieveContact(const Collider *other) const;

    bool isDirty() const;

protected:
    void createCollider() override;

    void update() override;

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

protected:
    typedef unordered_map<uint32_t, bool> CollisionMap;

    CollisionMap m_collisions;

    Vector3 m_center;

    PhysicMaterial *m_material;

    bool m_dirty;

    bool m_trigger;

};
typedef VolumeCollider* VolumeColliderPtr;

#endif // VOLUMECOLLIDER_H
