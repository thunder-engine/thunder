#ifndef CAPSULECOLLIDER_H
#define CAPSULECOLLIDER_H

#include "components/spherecollider.h"

class CapsuleCollider : public SphereCollider {
    A_REGISTER(CapsuleCollider, SphereCollider, Components)

    A_PROPERTIES(
        A_PROPERTY(float, Height, CapsuleCollider::height, CapsuleCollider::setHeight)
    )
    A_NOMETHODS()

public:
    CapsuleCollider ();

    float height () const;
    void setHeight (float height);

    btCollisionShape *shape() override;

private:
#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

protected:
    float m_Height;

};

#endif // CAPSULECOLLIDER_H
