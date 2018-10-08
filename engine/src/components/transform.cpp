#include "components/transform.h"

#include "components/actor.h"

Transform::Transform() :
        m_Dirty(true),
        m_Position(Vector3()),
        m_Euler(Vector3()),
        m_Rotation(Quaternion()),
        m_Scale(Vector3(1.0f)),
        m_Transform(Matrix4()) {

}

Vector3 Transform::position() const {
    return m_Position;
}

Vector3 Transform::euler() const {
    return m_Euler;
}

Quaternion Transform::rotation() const {
    return m_Rotation;
}

Vector3 Transform::scale() const {
    return m_Scale;
}


void Transform::setPosition(const Vector3 &value) {
    m_Position  = value;
    m_Dirty     = true;
}

void Transform::setEuler(const Vector3 &value) {
    m_Euler = value;
    setRotation(Quaternion(m_Euler));
}

void Transform::setRotation(const Quaternion &value) {
    m_Rotation  = value;
    m_Dirty     = true;
}

void Transform::setScale(const Vector3 &value) {
    m_Scale = value;
    m_Dirty = true;
}

Matrix4 Transform::worldTransform() {
    if(m_Dirty) {
        for(auto &it : actor().findChildren<Actor *>(false)) {
            it->transform()->m_Dirty = true;
        }

        m_Transform = Matrix4(m_Position, m_Rotation, m_Scale);
        Actor *cur  = dynamic_cast<Actor *>(actor().parent());
        while(cur) {
            Transform *t    = cur->transform();
            m_Transform     = t->worldTransform() * m_Transform;
            cur = dynamic_cast<Actor *>(cur->parent());
        }
        m_Dirty = false;
    }
    return m_Transform;
}

Vector3 Transform::worldPosition() const {
    Vector3 result  = m_Position;
    Actor *cur  = dynamic_cast<Actor *>(actor().parent());
    while(cur) {
        Transform *t    = cur->transform();
        result  = result * t->m_Scale;
        result += t->m_Position;
        result  = t->m_Rotation.toMatrix() * result;
        cur     = dynamic_cast<Actor *>(cur->parent());
    }
    return result;
}

Vector3 Transform::worldEuler() const {
    Vector3 result  = m_Euler;
    Actor *cur  = dynamic_cast<Actor *>(actor().parent());
    while(cur) {
        Transform *t    = cur->transform();
        result += t->m_Euler;
        cur     = dynamic_cast<Actor *>(cur->parent());
    }
    return result;
}

Quaternion Transform::worldRotation() const {
    Quaternion result   = m_Rotation;
    Actor *cur  = dynamic_cast<Actor *>(actor().parent());
    while(cur) {
        Transform *t    = cur->transform();
        result  = result * t->m_Rotation;
        cur     = dynamic_cast<Actor *>(cur->parent());
    }
    return result;
}

Vector3 Transform::worldScale() const {
    Vector3 result  = m_Scale;
    Actor *cur  = dynamic_cast<Actor *>(actor().parent());
    while(cur) {
        Transform *t    = cur->transform();
        result  = result * t->m_Scale;
        cur     = dynamic_cast<Actor *>(cur->parent());
    }
    return result;
}
