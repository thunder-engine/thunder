#ifndef SPHERECOLLIDER_H
#define SPHERECOLLIDER_H

#include "components/volumecollider.h"

class SphereCollider : public VolumeCollider {
    A_REGISTER(SphereCollider, VolumeCollider, Components)

    A_PROPERTIES(
        A_PROPERTY(float, radius, SphereCollider::radius, SphereCollider::setRadius)
    )
    A_NOMETHODS()

public:
    SphereCollider ();

    float radius () const;
    void setRadius (float radius);

    btCollisionShape *shape() override;

private:
#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

protected:
    float m_Radius;

};

#endif // SPHERECOLLIDER_H
