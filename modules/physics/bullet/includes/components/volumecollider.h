#ifndef VOLUMECOLLIDER_H
#define VOLUMECOLLIDER_H

#include "collider.h"

class PhysicMaterial;

class VolumeCollider : public Collider {
    A_REGISTER(VolumeCollider, Collider, General)

    A_PROPERTIES(
        A_PROPERTY(bool, Trigger, VolumeCollider::trigger, VolumeCollider::setTrigger),
        A_PROPERTYEX(PhysicMaterial *, Material, VolumeCollider::material, VolumeCollider::setMaterial, "editor=Template"),
        A_PROPERTY(Vector3, Center, VolumeCollider::center, VolumeCollider::setCenter)
    )

    A_METHODS(
        A_SIGNAL(VolumeCollider::entered),
        A_SIGNAL(VolumeCollider::stay),
        A_SIGNAL(VolumeCollider::exited)
    )

public:
     VolumeCollider();
    ~VolumeCollider() override;

    void update() override;

    bool trigger() const;
    void setTrigger(bool trigger);

    PhysicMaterial *material() const;
    void setMaterial(PhysicMaterial *material);

    Vector3 center() const;
    void setCenter(const Vector3 &center);

    void retrieveContact(const Collider *other) const;

    void entered();
    void stay();
    void exited();

protected:
    void createCollider() override;

private:
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

protected:
    bool m_Trigger;

    PhysicMaterial *m_pMaterial;

    Vector3 m_Center;

    typedef unordered_map<uint32_t, bool> CollisionMap;

    CollisionMap m_Collisions;

};

#endif // VOLUMECOLLIDER_H
