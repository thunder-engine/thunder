#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "component.h"

class TransformPrivate;

class NEXT_LIBRARY_EXPORT Transform : public Component {
    A_REGISTER(Transform, Component, Components)

    A_PROPERTIES(
        A_PROPERTY(Vector3, position, Transform::position, Transform::setPosition),
        A_PROPERTY(Vector3, rotation, Transform::rotation, Transform::setRotation),
        A_PROPERTY(Quaternion, quaternion, Transform::quaternion, Transform::setQuaternion),
        A_PROPERTY(Vector3, scale, Transform::scale, Transform::setScale)
    )
    A_METHODS(
        A_METHOD(Transform *, Transform::parentTransform),
        A_METHOD(void, Transform::setParentTransform),
        A_METHOD(Matrix4, Transform::localTransform),
        A_METHOD(Matrix4, Transform::worldTransform),
        A_METHOD(Vector3, Transform::worldPosition),
        A_METHOD(Vector3, Transform::worldEuler),
        A_METHOD(Quaternion, Transform::worldRotation),
        A_METHOD(Vector3, Transform::worldScale)
    )

public:
    Transform ();
    ~Transform ();

    Vector3 position () const;
    void setPosition (const Vector3 &position);

    Vector3 rotation () const;
    void setRotation (const Vector3 &angles);

    virtual Quaternion quaternion () const;
    void setQuaternion (const Quaternion &quaternion);

    Vector3 scale () const;
    void setScale (const Vector3 &scale);

    Transform *parentTransform () const;
    void setParentTransform (Transform *parent, bool force = false);

    Matrix4 &localTransform ();
    Matrix4 &worldTransform ();

    Vector3 worldPosition () const;
    Vector3 worldEuler () const;
    Quaternion worldRotation () const;
    Vector3 worldScale () const;

private:
    void setDirty ();
    void cleanDirty ();

private:
    TransformPrivate *p_ptr;

};

#endif // TRANSFORM_H
