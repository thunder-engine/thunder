#ifndef SPHERECOLLIDER_H
#define SPHERECOLLIDER_H

#include "volumecollider.h"

class BULLET_EXPORT SphereCollider : public VolumeCollider {
    A_OBJECT(SphereCollider, VolumeCollider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(float, radius, SphereCollider::radius, SphereCollider::setRadius)
    )
    A_NOMETHODS()

public:
    SphereCollider();

    float radius() const;
    void setRadius(float radius);

private:
    void drawGizmosSelected() override;

    btCollisionShape *shape() override;

protected:
    float m_radius;

};
typedef SphereCollider* SphereColliderPtr;

#endif // SPHERECOLLIDER_H
