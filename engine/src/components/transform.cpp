#include "components/transform.h"

#include "components/actor.h"

#include <algorithm>
#include <cstring>

/*!
    \class Transform
    \brief Position, rotation and scale of an Actor.
    \inmodule Components

    Every Actor in a Scene has a Transform.
    It's used to store and manipulate the position, rotation and scale of the object.
    Every Transform can have a parent, which allows you to apply position, rotation and scale hierarchically.
*/

static std::hash<float> hash_float;

Transform::Transform() :
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

Transform::Transform(const Transform &origin) :
        m_position(origin.m_position),
        m_rotation(origin.m_rotation),
        m_scale(origin.m_scale),
        m_worldRotation(origin.m_worldRotation),
        m_worldScale(origin.m_worldScale),
        m_quaternion(origin.m_quaternion),
        m_worldQuaternion(origin.m_worldQuaternion),
        m_transform(origin.m_transform),
        m_worldTransform(origin.m_worldTransform),
        m_parent(origin.m_parent),
        m_hash(origin.m_hash),
        m_dirty(origin.m_dirty) {

}

Transform::~Transform() {
    setParentTransform(nullptr, true);

    std::list<Transform *> temp = m_children;
    for(auto it : temp) {
        it->setParentTransform(nullptr, true);
    }
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
    if(m_position != position) {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_position = position;
        setDirty();
    }
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
    if(m_rotation != angles) {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_rotation = angles;
        m_quaternion = Quaternion(m_rotation);
        setDirty();
    }
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
    if(m_quaternion != quaternion) {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_quaternion = quaternion;
    #ifdef SHARED_DEFINE
        //m_rotation = m_quaternion.euler();
    #endif
        setDirty();
    }
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
    if(m_scale != scale) {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_scale = scale;
        setDirty();
    }
}
/*!
    Marks transform as dirty.
*/
void Transform::setDirty() {
    m_dirty = true;
    for(auto it : m_children) {
        it->setDirty();
    }
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
    if(parent == this) {
        return;
    }

    if(parent && actor()->parent() != parent->actor()) {
        actor()->setParent(parent->actor());
        return;
    }

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
const Matrix4 &Transform::localTransform() const {
    cleanDirty();
    return m_transform;
}
/*!
    Returns current transform matrix in world space.
*/
const Matrix4 &Transform::worldTransform() const {
    cleanDirty();
    return m_worldTransform;
}
/*!
    Returns current position of the transform in world space.
*/
Vector3 Transform::worldPosition() const {
    cleanDirty();
    return Vector3(m_worldTransform[12], m_worldTransform[13], m_worldTransform[14]);
}
/*!
    Returns current rotation of the transform in world space as Euler angles in degrees.
*/
Vector3 Transform::worldRotation() const {
    cleanDirty();
    return m_worldRotation;
}
/*!
    Returns current rotation of the transform in world space as Quaternion.
*/
Quaternion Transform::worldQuaternion() const {
    cleanDirty();
    return m_worldQuaternion;
}
/*!
    Returns current scale of the transform in world space.
*/
Vector3 Transform::worldScale() const {
    cleanDirty();
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
uint32_t Transform::hash() const {
    cleanDirty();
    return m_hash;
}
/*!
    \internal
*/
const std::list<Transform *> &Transform::children() const {
    return m_children;
}

void Transform::cleanDirty() const {
    if(m_dirty) {
        std::unique_lock<std::mutex> locker(m_mutex);

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
        m_hash = 16;
        for(int i = 0; i < 16; i++) {
            Mathf::hashCombine(m_hash, m_worldTransform[i]);
        }
        m_dirty = false;
    }
}
