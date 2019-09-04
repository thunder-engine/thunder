#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#include "components/volumecollider.h"

class BoxCollider : public VolumeCollider {
    A_REGISTER(BoxCollider, VolumeCollider, Components)

    A_PROPERTIES(
        A_PROPERTY(bool, Size, BoxCollider::size, BoxCollider::setSize)
    )
    A_NOMETHODS()

public:
    BoxCollider ();

    Vector3 size () const;
    void setSize (const Vector3 &size);

    btCollisionShape *shape() override;

private:
#ifdef NEXT_SHARED
    bool drawHandles(bool selected) override;
#endif

protected:
    Vector3 m_Size;

};

#endif // BOXCOLLIDER_H
