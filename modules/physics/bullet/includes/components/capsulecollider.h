#ifndef CAPSULECOLLIDER_H
#define CAPSULECOLLIDER_H

#include "spherecollider.h"

class BULLET_EXPORT CapsuleCollider : public SphereCollider {
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
#ifdef SHARED_DEFINE
    bool drawHandles(ObjectList &selected) override;
#endif

    btCollisionShape *shape() override;

protected:
    float m_height;

};
typedef CapsuleCollider* CapsuleColliderPtr;

#endif // CAPSULECOLLIDER_H
