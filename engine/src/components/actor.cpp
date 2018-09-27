#include "components/actor.h"

#include "components/transform.h"

Actor::Actor() :
        m_pScene(nullptr),
        m_Enable(true),
        m_Layers(0x17),
        m_pTransform(nullptr) {

}

bool Actor::isEnable() const {
    return m_Enable;
}

void Actor::setEnable(const bool enable) {
    m_Enable    = enable;
}

uint8_t Actor::layers() const {
    return m_Layers;
}

Scene &Actor::scene() const {
    return *m_pScene;
}

Transform *Actor::transform() {
    if(m_pTransform == nullptr) {
        m_pTransform    = component<Transform>();
    }
    return m_pTransform;
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
    Transform *t    = transform();
    if(t) {
        Vector3 p   = t->worldPosition();
        Vector3 e   = t->worldEuler();
        Vector3 s   = t->worldScale();

        Object::setParent(parent);

        Actor *actor   = dynamic_cast<Actor *>(parent);
        if(actor) {
            Transform *par  = actor->transform();
            if(par) {
                Vector3 scale   = par->worldScale();
                scale   = Vector3(1.0 / scale.x, 1.0 / scale.y, 1.0 / scale.z);

                p   = par->worldRotation().inverse() * ((p - par->worldPosition()) * scale);
                e   = e - par->worldEuler();
                s   = s * scale;
            }
        }

        t->setPosition(p);
        t->setEuler(e);
        t->setScale(s);
    } else {
        Object::setParent(parent);
    }
}
