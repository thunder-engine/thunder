#ifndef CAPSULECOLLIDER_H
#define CAPSULECOLLIDER_H

#include "components/spherecollider.h"

class CapsuleCollider : public SphereCollider {
    A_REGISTER(CapsuleCollider, SphereCollider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(float, height, CapsuleCollider::height, CapsuleCollider::setHeight)
    )
    A_NOMETHODS()

public:
    CapsuleCollider();

    float height() const;
    void setHeight(float height);

private:
#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

    btCollisionShape *shape() override;

protected:
    float m_Height;

};

#endif // CAPSULECOLLIDER_H
