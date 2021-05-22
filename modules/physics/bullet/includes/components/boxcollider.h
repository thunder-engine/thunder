#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#include "components/volumecollider.h"

class BoxCollider : public VolumeCollider {
    A_REGISTER(BoxCollider, VolumeCollider, Components/Physics)

    A_PROPERTIES(
        A_PROPERTY(Vector3, size, BoxCollider::size, BoxCollider::setSize)
    )
    A_NOMETHODS()

public:
    BoxCollider();

    const Vector3 &size() const;
    void setSize(const Vector3 &size);

private:
#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

    btCollisionShape *shape() override;

protected:
    Vector3 m_Size;

};

#endif // BOXCOLLIDER_H
