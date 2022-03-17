#include "components/transform.h"

#include "components/actor.h"

#include <algorithm>
#include <mutex>

class TransformPrivate {
public:
    TransformPrivate() :
        m_Position(Vector3()),
        m_Rotation(Vector3()),
        m_Scale(Vector3(1.0f)),
        m_WorldPosition(Vector3()),
        m_WorldRotation(Vector3()),
        m_WorldScale(Vector3(1.0f)),
        m_Quaternion(Quaternion()),
        m_WorldQuaternion(Quaternion()),
        m_Transform(Matrix4()),
        m_WorldTransform(Matrix4()),
        m_pParent(nullptr),
        m_Dirty(true) {

    }

    void cleanDirty() {
        unique_lock<mutex> locker(m_Mutex);
        m_Transform = Matrix4(m_Position, m_Quaternion, m_Scale);
        m_WorldTransform = m_Transform;
        m_WorldRotation = m_Rotation;
        m_WorldPosition = m_Position;
        m_WorldQuaternion = m_Quaternion;
        m_WorldScale = m_Scale;
        if(m_pParent) {
            m_WorldPosition = m_pParent->worldTransform() * m_WorldPosition;
            m_WorldScale = m_pParent->worldScale() * m_WorldScale;
            m_WorldRotation = m_pParent->worldRotation() + m_WorldRotation;
            m_WorldQuaternion = m_pParent->worldQuaternion() * m_WorldQuaternion;
            m_WorldTransform = m_pParent->worldTransform() * m_WorldTransform;
        }
        m_Dirty = false;
    }

    Vector3 m_Position;
    Vector3 m_Rotation;
    Vector3 m_Scale;

    Vector3 m_WorldPosition;
    Vector3 m_WorldRotation;
    Vector3 m_WorldScale;

    Quaternion m_Quaternion;
    Quaternion m_WorldQuaternion;

    Matrix4 m_Transform;
    Matrix4 m_WorldTransform;

    list<Transform *> m_Children;

    Transform *m_pParent;

    mutex m_Mutex;

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
    setParentTransform(nullptr, true);

    list<Transform *> temp = p_ptr->m_Children;
    for(auto it : temp) {
        it->setParentTransform(nullptr, true);
    }

    delete p_ptr;
    p_ptr = nullptr;
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
void Transform::setPosition(const Vector3 position) {
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    p_ptr->m_Position = position;
    setDirty();
}
/*!
    Returns current rotation of the Transform in local space as Euler angles in degrees.
*/
Vector3 &Transform::rotation() const {
    return p_ptr->m_Rotation;
}
/*!
    Changes the rotation of the Transform in local space by provided Euler \a angles in degrees.
*/
void Transform::setRotation(const Vector3 angles) {
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    p_ptr->m_Rotation = angles;
    p_ptr->m_Quaternion = Quaternion(p_ptr->m_Rotation);
    setDirty();
}
/*!
    Returns current rotation of the Transform in local space as Quaternion.
*/
Quaternion &Transform::quaternion() const {
    return p_ptr->m_Quaternion;
}
/*!
    Changes the rotation \a quaternion of the Transform in local space by provided Quaternion.
*/
void Transform::setQuaternion(const Quaternion quaternion) {
    unique_lock<mutex> locker(p_ptr->m_Mutex);
    p_ptr->m_Quaternion = quaternion;
#ifdef SHARED_DEFINE
    //p_ptr->m_Rotation = p_ptr->m_Quaternion.euler();
#endif
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
void Transform::setScale(const Vector3 scale) {
    unique_lock<mutex> locker(p_ptr->m_Mutex);
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
    Vector3 p;
    Vector3 e;
    Vector3 s;

    if(parent) {
        p = worldPosition();
        e = worldRotation();
        s = worldScale();
    }

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

            p_ptr->m_Position = p_ptr->m_pParent->worldQuaternion().inverse() * ((p - p_ptr->m_pParent->worldPosition()) * scale);
            p_ptr->m_Scale = s * scale;
            setRotation(e - p_ptr->m_pParent->worldRotation());
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
Vector3 &Transform::worldPosition() const {
    if(p_ptr->m_Dirty) {
        p_ptr->cleanDirty();
    }
    return p_ptr->m_WorldPosition;
}
/*!
    Returns current rotation of the transform in world space as Euler angles in degrees.
*/
Vector3 &Transform::worldRotation() const {
    if(p_ptr->m_Dirty) {
        p_ptr->cleanDirty();
    }
    return p_ptr->m_WorldRotation;
}
/*!
    Returns current rotation of the transform in world space as Quaternion.
*/
Quaternion &Transform::worldQuaternion() const {
    if(p_ptr->m_Dirty) {
        p_ptr->cleanDirty();
    }
    return p_ptr->m_WorldQuaternion;
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
    Makes the Transform a child of \a parent at given \a position.
    \note Please ignore the \a force flag it will be provided by the default.
*/
void Transform::setParent(Object *parent, int32_t position, bool force) {
    A_UNUSED(position);
    if(parent == this) {
        return;
    }
    Object::setParent(parent, 0, force);

    Actor *p = dynamic_cast<Actor *>(actor()->parent());
    if(p) {
        setParentTransform(p->transform(), true);
    }
}
/*!
    \internal
*/
list<Transform *> &Transform::children() const {
    return p_ptr->m_Children;
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
