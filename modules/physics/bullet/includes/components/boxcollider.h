#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#include "components/volumecollider.h"

class BoxCollider : public VolumeCollider {
    A_REGISTER(BoxCollider, VolumeCollider, Components)

    A_PROPERTIES(
        A_PROPERTY(Vector3, size, BoxCollider::size, BoxCollider::setSize)
    )
    A_NOMETHODS()

public:
    BoxCollider();

    const Vector3 &size() const;
    void setSize(const Vector3 &size);

    btCollisionShape *shape() override;

private:
#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

protected:
    Vector3 m_Size;

};

#endif // BOXCOLLIDER_H
