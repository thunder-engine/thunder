#include "components/transform.h"

#include "components/actor.h"

#include <algorithm>
#include <cstring>

class TransformPrivate {
public:
    TransformPrivate()  {

    }

    mutex m_mutex;

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
    p_ptr(new TransformPrivate),
    m_position(Vector3()),
    m_rotation(Vector3()),
    m_scale(Vector3(1.0f)),
    m_worldRotation(Vector3()),
    m_worldScale(Vector3(1.0f)),
    m_quaternion(Quaternion()),
    m_worldQuaternion(Quaternion()),
    m_transform(Matrix4()),
    m_worldTransform(Matrix4()),
    m_parent(nullptr),
    m_hash(0),
    m_dirty(true) {

}

Transform::~Transform() {
    setParentTransform(nullptr, true);

    list<Transform *> temp = m_children;
    for(auto it : temp) {
        it->setParentTransform(nullptr, true);
    }

    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    Returns current position of the Transform in local space.
*/
Vector3 Transform::position() const {
    return m_position;
}
/*!
    Changes \a position of the Transform in local space.
*/
void Transform::setPosition(const Vector3 position) {
    unique_lock<mutex> locker(p_ptr->m_mutex);
    m_position = position;
    setDirty();
}
/*!
    Returns current rotation of the Transform in local space as Euler angles in degrees.
*/
Vector3 Transform::rotation() const {
    return m_rotation;
}
/*!
    Changes the rotation of the Transform in local space by provided Euler \a angles in degrees.
*/
void Transform::setRotation(const Vector3 angles) {
    unique_lock<mutex> locker(p_ptr->m_mutex);
    m_rotation = angles;
    m_quaternion = Quaternion(m_rotation);
    setDirty();
}
/*!
    Returns current rotation of the Transform in local space as Quaternion.
*/
Quaternion Transform::quaternion() const {
    return m_quaternion;
}
/*!
    Changes the rotation \a quaternion of the Transform in local space by provided Quaternion.
*/
void Transform::setQuaternion(const Quaternion quaternion) {
    unique_lock<mutex> locker(p_ptr->m_mutex);
    m_quaternion = quaternion;
#ifdef SHARED_DEFINE
    //m_rotation = m_quaternion.euler();
#endif
    setDirty();
}
/*!
    Returns current scale of the Transform in local space.
*/
Vector3 Transform::scale() const {
    return m_scale;
}
/*!
    Changes the \a scale of the Transform in local space.
*/
void Transform::setScale(const Vector3 scale) {
    unique_lock<mutex> locker(p_ptr->m_mutex);
    m_scale = scale;
    setDirty();
}
/*!
    Returns parent of the transform.
*/
Transform *Transform::parentTransform() const {
    return m_parent;
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

    if(m_parent) {
        auto it = std::find(m_parent->m_children.begin(),
                            m_parent->m_children.end(),
                            this);
        if(it != m_parent->m_children.end()) {
            m_parent->m_children.erase(it);
        }
    }

    m_parent = parent;
    if(m_parent) {
        m_parent->m_children.push_back(this);
        if(!force) {
            Vector3 scale = m_parent->worldScale();
            scale = Vector3(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z);

            m_position = m_parent->worldQuaternion().inverse() * ((p - m_parent->worldPosition()) * scale);
            m_scale = s * scale;
            setRotation(e - m_parent->worldRotation());
        } else {
            setDirty();
        }
    }
}
/*!
    Returns current transform matrix in local space.
*/
Matrix4 Transform::localTransform() const {
    if(m_dirty) {
        unique_lock<mutex> locker(p_ptr->m_mutex);
        cleanDirty();
    }
    return m_transform;
}
/*!
    Returns current transform matrix in world space.
*/
Matrix4 Transform::worldTransform() const {
    if(m_dirty) {
        unique_lock<mutex> locker(p_ptr->m_mutex);
        cleanDirty();
    }
    return m_worldTransform;
}
/*!
    Returns current position of the transform in world space.
*/
Vector3 Transform::worldPosition() const {
    if(m_dirty) {
        unique_lock<mutex> locker(p_ptr->m_mutex);
        cleanDirty();
    }
    return Vector3(m_worldTransform[12], m_worldTransform[13], m_worldTransform[14]);
}
/*!
    Returns current rotation of the transform in world space as Euler angles in degrees.
*/
Vector3 Transform::worldRotation() const {
    if(m_dirty) {
        unique_lock<mutex> locker(p_ptr->m_mutex);
        cleanDirty();
    }
    return m_worldRotation;
}
/*!
    Returns current rotation of the transform in world space as Quaternion.
*/
Quaternion Transform::worldQuaternion() const {
    if(m_dirty) {
        unique_lock<mutex> locker(p_ptr->m_mutex);
        cleanDirty();
    }
    return m_worldQuaternion;
}
/*!
    Returns current scale of the transform in world space.
*/
Vector3 Transform::worldScale() const {
    if(m_dirty) {
        unique_lock<mutex> locker(p_ptr->m_mutex);
        cleanDirty();
    }
    return m_worldScale;
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
int Transform::hash() const {
    return m_hash;
}
/*!
    \internal
*/
const list<Transform *> &Transform::children() const {
    return m_children;
}
/*!
    \internal
*/
void Transform::setDirty() {
    m_dirty = true;
    for(auto it : m_children) {
        it->setDirty();
    }
}

void Transform::cleanDirty() const {
    m_transform = Matrix4(m_position, m_quaternion, m_scale);
    m_worldTransform = m_transform;
    m_worldRotation = m_rotation;
    m_worldQuaternion = m_quaternion;
    m_worldScale = m_scale;
    if(m_parent) {
        m_worldScale = m_parent->worldScale() * m_worldScale;
        m_worldRotation = m_parent->worldRotation() + m_worldRotation;
        m_worldQuaternion = m_parent->worldQuaternion() * m_worldQuaternion;
        m_worldTransform = m_parent->worldTransform() * m_worldTransform;
    }
    int32_t buffer[16];
    memcpy(buffer, &m_worldTransform[0], sizeof(float) * 16);
    m_hash = 0;
    for(int i = 0; i < 16; i++) {
        m_hash ^= buffer[i];
    }
    m_dirty = false;
}
