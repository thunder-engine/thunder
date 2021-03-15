#include "components/transform.h"

#include "components/actor.h"

#include <algorithm>

class TransformPrivate {
public:
    TransformPrivate() :
        m_Position(Vector3()),
        m_Euler(Vector3()),
        m_Scale(Vector3(1.0f)),
        m_WorldPosition(Vector3()),
        m_WorldEuler(Vector3()),
        m_WorldScale(Vector3(1.0f)),
        m_Rotation(Quaternion()),
        m_WorldRotation(Quaternion()),
        m_Transform(Matrix4()),
        m_WorldTransform(Matrix4()),
        m_pParent(nullptr),
        m_Dirty(true) {

    }

    void cleanDirty() {
        m_Transform = Matrix4(m_Position, m_Rotation, m_Scale);
        m_WorldTransform = m_Transform;
        if(m_pParent) {

            m_WorldEuler = m_pParent->worldEuler() + m_Euler;
            m_WorldRotation = m_pParent->worldRotation() * m_Rotation;
            m_WorldTransform = m_pParent->worldTransform() * m_WorldTransform;
        }
        m_Dirty = false;
    }

    Vector3 m_Position;
    Vector3 m_Euler;
    Vector3 m_Scale;

    Vector3 m_WorldPosition;
    Vector3 m_WorldEuler;
    Vector3 m_WorldScale;

    Quaternion m_Rotation;
    Quaternion m_WorldRotation;

    Matrix4 m_Transform;
    Matrix4 m_WorldTransform;

    list<Transform *> m_Children;

    Transform *m_pParent;

    bool m_Dirty;
};
/*!
    \class Transform
    \brief Position, rotation and scale of an Actor.
    \inmodule Engine

    Every Actor in a Scene has a Transform.
    It's used to store and manipulate the position, rotation and scale of the object.
    Every Transform can have a parent, which allows you to apply position, rotation and scale hierarchically.
*/

Transform::Transform() :
        p_ptr(new TransformPrivate) {
}

Transform::~Transform() {
    delete p_ptr;
}
/*!
    Returns current position of the Transform in local space.
*/
Vector3 &Transform::position() const {
    return p_ptr->m_Position;
}
/*!
    Changes \a position of the Transform in local space.
*/
void Transform::setPosition(const Vector3 &position) {
    p_ptr->m_Position = position;
    setDirty();
}
/*!
    Returns current rotation of the Transform in local space as Euler angles in degrees.
*/
Vector3 &Transform::rotation() const {
    return p_ptr->m_Euler;
}
/*!
    Changes the rotation of the Transform in local space by provided Euler \a angles in degrees.
*/
void Transform::setRotation(const Vector3 &angles) {
    p_ptr->m_Euler = angles;
    setQuaternion(Quaternion(p_ptr->m_Euler));
}
/*!
    Returns current rotation of the Transform in local space as Quaternion.
*/
Quaternion &Transform::quaternion() const {
    return p_ptr->m_Rotation;
}
/*!
    Changes the rotation \a quaternion of the Transform in local space by provided Quaternion.
*/
void Transform::setQuaternion(const Quaternion &quaternion) {
    p_ptr->m_Rotation = quaternion;
    setDirty();
}
/*!
    Returns current scale of the Transform in local space.
*/
Vector3 &Transform::scale() const {
    return p_ptr->m_Scale;
}
/*!
    Changes the \a scale of the Transform in local space.
*/
void Transform::setScale(const Vector3 &scale) {
    p_ptr->m_Scale = scale;
    setDirty();
}
/*!
    Returns parent of the transform.
*/
Transform *Transform::parentTransform() const {
    return p_ptr->m_pParent;
}
/*!
    Changing the \a parent will modify the parent-relative position, scale and rotation but keep the world space position, rotation and scale the same.
    In case of \a force flag provided as true, no recalculations of transform happen.
*/
void Transform::setParentTransform(Transform *parent, bool force) {
    Vector3 p = worldPosition();
    Vector3 e = worldEuler();
    Vector3 s = worldScale();

    if(p_ptr->m_pParent) {
        auto it = std::find(p_ptr->m_pParent->p_ptr->m_Children.begin(),
                            p_ptr->m_pParent->p_ptr->m_Children.end(),
                            this);
        if(it != p_ptr->m_pParent->p_ptr->m_Children.end()) {
            p_ptr->m_pParent->p_ptr->m_Children.erase(it);
        }
    }

    p_ptr->m_pParent = parent;
    if(p_ptr->m_pParent) {
        p_ptr->m_pParent->p_ptr->m_Children.push_back(this);
        if(!force) {
            Vector3 scale = p_ptr->m_pParent->worldScale();
            scale = Vector3(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z);

            p_ptr->m_Position = p_ptr->m_pParent->worldRotation().inverse() * ((p - p_ptr->m_pParent->worldPosition()) * scale);
            p_ptr->m_Scale = s * scale;
            setRotation(e - p_ptr->m_pParent->worldEuler());
        } else {
            setDirty();
        }
    }
}
/*!
    Returns current transform matrix in local space.
*/
Matrix4 &Transform::localTransform() const {
    if(p_ptr->m_Dirty) {
        p_ptr->cleanDirty();
    }
    return p_ptr->m_Transform;
}
/*!
    Returns current transform matrix in world space.
*/
Matrix4 &Transform::worldTransform() const {
    if(p_ptr->m_Dirty) {
        p_ptr->cleanDirty();
    }
    return p_ptr->m_WorldTransform;
}
/*!
    Returns current position of the transform in world space.
*/
Vector3 Transform::worldPosition() const {
    Vector3 result = p_ptr->m_Position;
    Transform *cur = p_ptr->m_pParent;
    while(cur) {
        result = result * cur->p_ptr->m_Scale;
        result = cur->p_ptr->m_Rotation.toMatrix() * result;
        result += cur->p_ptr->m_Position;
        cur = cur->parentTransform();
    }
    return result;
}
/*!
    Returns current rotation of the transform in world space as Euler angles in degrees.
*/
Vector3 &Transform::worldEuler() const {
    if(p_ptr->m_Dirty) {
        p_ptr->cleanDirty();
    }
    return p_ptr->m_WorldEuler;
}
/*!
    Returns current rotation of the transform in world space as Quaternion.
*/
Quaternion &Transform::worldRotation() const {
    if(p_ptr->m_Dirty) {
        p_ptr->cleanDirty();
    }
    return p_ptr->m_WorldRotation;
}
/*!
    Returns current scale of the transform in world space.
*/
Vector3 &Transform::worldScale() const {
    if(p_ptr->m_Dirty) {
        p_ptr->cleanDirty();
    }
    return p_ptr->m_WorldScale;
}

/*!
    \internal
*/
void Transform::setDirty() {
    p_ptr->m_Dirty = true;
    for(auto it : p_ptr->m_Children) {
        it->setDirty();
    }
}
