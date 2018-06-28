#include "components/actor.h"

#include "components/component.h"

Actor::Actor() :
        m_pScene(nullptr),
        m_Enable(true),
        m_Position(Vector3()),
        m_Rotation(Quaternion()),
        m_Scale(Vector3(1.0f)),
        m_Layers(0x17) {

}

bool Actor::isEnable() const {
    return m_Enable;
}

void Actor::setEnable(const bool enable) {
    m_Enable    = enable;
}

Matrix4 Actor::transform() {
    return m_Transform;
}

Vector3 Actor::position() const {
    return m_Position;
}

Vector3 Actor::euler() const {
    return m_Euler;
}

Quaternion Actor::rotation() const {
    return m_Rotation;
}

Vector3 Actor::scale() const {
    return m_Scale;
}

Matrix4 Actor::worldTransform() {
    Actor *p    = dynamic_cast<Actor *>(parent());
    if(p) {
        return p->worldTransform() * m_Transform;
    }
    return m_Transform;
}

Vector3 Actor::worldPosition() const {
    Actor *p    = dynamic_cast<Actor *>(parent());
    if(p) {
        return p->rotation().toMatrix() * (p->worldPosition() + m_Position) * p->scale();
    }
    return m_Position;
}

Vector3 Actor::worldEuler() const {
    Actor *p    = dynamic_cast<Actor *>(parent());
    if(p) {
        return p->worldEuler() + m_Euler;
    }
    return m_Euler;
}

Quaternion Actor::worldRotation() const {
    Actor *p  = dynamic_cast<Actor *>(parent());
    if(p) {
        return p->worldRotation() * m_Rotation;
    }
    return m_Rotation;
}

Vector3 Actor::worldScale() const {
    Actor *p  = dynamic_cast<Actor *>(parent());
    if(p) {
        return p->worldScale() * m_Scale;
    }
    return m_Scale;
}

uint8_t Actor::layers() const {
    return m_Layers;
}

Scene &Actor::scene() const {
    return *m_pScene;
}

Component *Actor::component(const char *type) {
    for(auto it : getChildren()) {
        const MetaObject *meta = it->metaObject();
        if(meta->canCastTo(type)) {
            return static_cast<Component *>(it);
        }
    }
    return nullptr;
}

void Actor::setPosition(const Vector3 &value) {
    m_Position  = value;
    m_Transform = Matrix4(m_Position, m_Rotation, m_Scale);
}

void Actor::setEuler(const Vector3 &value) {
    m_Euler = value;
    setRotation(Quaternion(m_Euler));
}

void Actor::setRotation(const Quaternion &value) {
    m_Rotation  = value;
    m_Transform = Matrix4(m_Position, m_Rotation, m_Scale);
}

void Actor::setScale(const Vector3 &value) {
    m_Scale     = value;
    m_Transform = Matrix4(m_Position, m_Rotation, m_Scale);
}

void Actor::setLayers(const uint8_t layers) {
    m_Layers    = layers;
}

void Actor::setScene(Scene &scene) {
    m_pScene = &scene;
}

Component *Actor::addComponent(const string &name) {
    return static_cast<Component *>(Engine::objectCreate(name, name, this));
}

void Actor::setParent(Object *parent) {
    Vector3 p   = worldPosition();
    Vector3 e   = worldEuler();
    Vector3 s   = worldScale();

    Object::setParent(parent);

    Actor *actor   = dynamic_cast<Actor *>(parent);
    if(actor) {
        p   = actor->worldRotation().inverse() * (p - actor->worldPosition()) * s;
        e   = e - actor->worldEuler();
        s   = s * actor->worldScale();
    }

    setPosition(p);
    setEuler(e);
    setScale(s);
}
