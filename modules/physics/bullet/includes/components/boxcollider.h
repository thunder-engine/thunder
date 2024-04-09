#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#include "volumecollider.h"

class BULLET_EXPORT BoxCollider : public VolumeCollider {
    A_REGISTER(BoxCollider, VolumeCollider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(Vector3, size, BoxCollider::size, BoxCollider::setSize)
    )
    A_NOMETHODS()

public:
    BoxCollider();

    const Vector3 &size() const;
    void setSize(const Vector3 size);

private:
    void drawGizmosSelected() override;

    btCollisionShape *shape() override;

protected:
    Vector3 m_size;

};
typedef BoxCollider *BoxColliderPtr;

#endif // BOXCOLLIDER_H
