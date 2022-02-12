#ifndef SPHERECOLLIDER_H
#define SPHERECOLLIDER_H

#include "components/volumecollider.h"

class SphereCollider : public VolumeCollider {
    A_REGISTER(SphereCollider, VolumeCollider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(float, radius, SphereCollider::radius, SphereCollider::setRadius)
    )
    A_NOMETHODS()

public:
    SphereCollider();

    float radius() const;
    void setRadius(float radius);

private:
#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif

    btCollisionShape *shape() override;

protected:
    float m_Radius;

};

#endif // SPHERECOLLIDER_H
