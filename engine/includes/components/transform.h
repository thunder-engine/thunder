#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "component.h"

class TransformPrivate;

class ENGINE_EXPORT Transform : public Component {
    A_REGISTER(Transform, Component, General)

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
        A_METHOD(Vector3, Transform::worldRotation),
        A_METHOD(Quaternion, Transform::worldQuaternion),
        A_METHOD(Vector3, Transform::worldScale)
    )

public:
    Transform();
    ~Transform();

    Vector3 &position() const;
    void setPosition(const Vector3 &position);

    Vector3 &rotation() const;
    void setRotation(const Vector3 &angles);

    virtual Quaternion &quaternion() const;
    void setQuaternion(const Quaternion &quaternion);

    Vector3 &scale() const;
    void setScale(const Vector3 &scale);

    Transform *parentTransform() const;
    virtual void setParentTransform(Transform *parent, bool force = false);

    virtual Matrix4 &localTransform() const;
    virtual Matrix4 &worldTransform() const;

    Vector3 &worldPosition() const;
    Vector3 &worldRotation() const;
    Quaternion &worldQuaternion() const;
    Vector3 &worldScale() const;

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

protected:
    list<Transform *> &children() const;

protected:
    virtual void setDirty();

private:
    friend class TransformPrivate;

    TransformPrivate *p_ptr;

};

#endif // TRANSFORM_H
