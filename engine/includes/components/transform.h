#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "component.h"

class TransformPrivate;

class NEXT_LIBRARY_EXPORT Transform : public Component {
    A_REGISTER(Transform, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(Vector3, Position, Transform::position, Transform::setPosition),
        A_PROPERTY(Vector3, Rotation, Transform::euler, Transform::setEuler),
        A_PROPERTY(Vector3, Scale, Transform::scale, Transform::setScale)
    )
    A_NOMETHODS()

public:
    Transform ();
    ~Transform ();

    Vector3 position () const;
    void setPosition (const Vector3 &position);

    Vector3 euler () const;
    void setEuler (const Vector3 &angles);

    virtual Quaternion rotation () const;
    void setRotation (const Quaternion &rotation);

    Vector3 scale () const;
    void setScale (const Vector3 &scale);

    Transform *parentTransform () const;
    void setParentTransform (Transform *parent, bool force = false);

    Matrix4 &worldTransform ();

    Vector3 worldPosition () const;
    Vector3 worldEuler () const;
    Quaternion worldRotation () const;
    Vector3 worldScale () const;

private:
    void setDirty ();

private:
    TransformPrivate *p_ptr;

};

#endif // TRANSFORM_H
