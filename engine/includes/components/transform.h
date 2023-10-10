#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "component.h"

#include <mutex>

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
    Transform(const Transform &origin);
    ~Transform();

    Vector3 position() const;
    void setPosition(const Vector3 position);

    Vector3 rotation() const;
    void setRotation(const Vector3 angles);

    Quaternion quaternion() const;
    void setQuaternion(const Quaternion quaternion);

    Vector3 scale() const;
    void setScale(const Vector3 scale);

    bool dirty() const;
    virtual void setDirty(bool dirty);

    Transform *parentTransform() const;
    void setParentTransform(Transform *parent, bool force = false);

    virtual Matrix4 localTransform() const;
    virtual Matrix4 worldTransform() const;

    Vector3 worldPosition() const;
    Vector3 worldRotation() const;
    Quaternion worldQuaternion() const;
    Vector3 worldScale() const;

    void setParent(Object *parent, int32_t position = -1, bool force = false) override;

    int hash() const;

protected:
    const list<Transform *> &children() const;

    virtual void cleanDirty() const;

protected:
    Vector3 m_position;
    Vector3 m_rotation;
    Vector3 m_scale;

    mutable Vector3 m_worldRotation;
    mutable Vector3 m_worldScale;

    Quaternion m_quaternion;
    mutable Quaternion m_worldQuaternion;

    mutable Matrix4 m_transform;
    mutable Matrix4 m_worldTransform;

    list<Transform *> m_children;

    Transform *m_parent;

    mutable mutex m_mutex;

    mutable int32_t m_hash;
    mutable bool m_dirty;

};

#endif // TRANSFORM_H
