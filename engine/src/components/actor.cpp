#include "components/actor.h"

#include "components/component.h"

Actor::Actor() :
        m_pScene(nullptr) {

    m_Position  = Vector3(0.0f);
    m_Rotation  = Quaternion ();
    m_Scale     = Vector3(1.0f);

    m_Layers    = 0x17;
}

Matrix4 Actor::transform() {
    return Matrix4(m_Position, m_Rotation, m_Scale);
}

Vector3 Actor::position() const {
    return m_Position;
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
        return p->worldTransform() * transform();
    }
    return transform();
}

Vector3 Actor::worldPosition() const {
    Actor *p    = dynamic_cast<Actor *>(parent());
    if(p) {
        return p->rotation().toMatrix() * (p->worldPosition() + position()) * p->scale();
    }
    return position();
}

Quaternion  Actor::worldRotation() const {
    Actor *p  = dynamic_cast<Actor *>(parent());
    if(p) {
        return p->worldRotation() * rotation();
    }
    return rotation();
}

Vector3 Actor::worldScale() const {
    Actor *p  = dynamic_cast<Actor *>(parent());
    if(p) {
        return p->worldScale() * scale();
    }
    return scale();
}

uint8_t Actor::layers() const {
    return m_Layers;
}

Scene &Actor::scene() const {
    return *m_pScene;
}

Component *Actor::component(const char *type) {
    for(auto it : getChildren()) {
        const AMetaObject *meta = it->metaObject();
        if(meta->canCastTo(type)) {
            return static_cast<Component *>(it);
        }
    }
    return nullptr;
}

void Actor::setPosition(const Vector3 &value) {
    m_Position  = value;
}

void Actor::setRotation(const Quaternion  &value) {
    m_Rotation  = value;
}

void Actor::setScale(const Vector3 &value) {
    m_Scale     = value;
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

void Actor::setParent(AObject *parent) {
    Vector3 p     = worldPosition();
    Quaternion  r   = worldRotation();
    Vector3 s     = worldScale();

    AObject::setParent(parent);

    Actor *actor   = dynamic_cast<Actor *>(parent);
    if(actor) {
        p   = actor->worldRotation().inverse() * (p - actor->worldPosition()) * s;
        r   = r * actor->worldRotation().inverse();
        s   = s * actor->worldScale();
    }

    setPosition(p);
    setRotation(r);
    setScale(s);
}
