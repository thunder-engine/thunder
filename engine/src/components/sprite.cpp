#include "components/sprite.h"

#include "resources/material.h"

#define MATERIAL    "Material"
#define BASEMAP     "BaseMap"

Sprite::Sprite() {
    m_Center    = Vector2();
    m_Material  = nullptr;
    m_Texture   = nullptr;
}

void Sprite::update() {
}

Vector2 Sprite::center() const {
    return m_Center;
}

void Sprite::setCenter(const Vector2 &value) {
    m_Center    = value;
}

Material *Sprite::material() const {
    return (m_Material) ? m_Material->material() : nullptr;
}

void Sprite::setMaterial(Material *material) {
    if(material) {
        m_Material  = material->createInstance();
    }
}

Texture *Sprite::texture() const {
    return m_Texture;
}

void Sprite::setTexture(Texture *texture) {
    m_Texture   = texture;
}

void Sprite::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(MATERIAL);
        if(it != data.end()) {
            setMaterial(Engine::loadResource<Material>((*it).second.toString()));
        }
    }
    {
        auto it = data.find(BASEMAP);
        if(it != data.end()) {
            setTexture(Engine::loadResource<Texture>((*it).second.toString()));
        }
    }
}

VariantMap Sprite::saveUserData() const {
    VariantMap result   = Component::saveUserData();
    {
        Material *m = material();
        string ref  = Engine::reference(m);
        if(!ref.empty()) {
            result[MATERIAL]    = ref;
        }
    }
    {
        Texture *t  = texture();
        string ref  = Engine::reference(t);
        if(!ref.empty()) {
            result[BASEMAP]    = ref;
        }
    }
    return result;
}
