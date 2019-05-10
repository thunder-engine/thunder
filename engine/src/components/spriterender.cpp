#include "components/spriterender.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"
#include "resources/mesh.h"

#include "commandbuffer.h"

#define MATERIAL    "Material"
#define BASEMAP     "BaseMap"

#define OVERRIDE "uni.texture0"

SpriteRender::SpriteRender() {
    m_Center    = Vector2();
    m_Texture   = nullptr;
    m_pMaterial = nullptr;
    m_pMesh     = Engine::loadResource<Mesh>(".embedded/plane.fbx");
}

void SpriteRender::draw(ICommandBuffer &buffer, int8_t layer) {
    Actor *a    = actor();
    if(m_pMesh && layer & a->layers()) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(ICommandBuffer::idToColor(a->uuid()));
        }

        for(uint32_t s = 0; s < m_pMesh->surfacesCount(); s++) {
            buffer.drawMesh(a->transform()->worldTransform(), m_pMesh, s, layer, m_pMaterial);
        }
        buffer.setColor(Vector4(1.0f));
    }
}

Material *SpriteRender::material() const {
    if(m_pMaterial) {
        return m_pMaterial->material();
    }
    return nullptr;
}

void SpriteRender::setMaterial(Material *material) {
    if(m_pMaterial) {
        delete m_pMaterial;
        m_pMaterial = nullptr;
    }

    if(material) {
        m_pMaterial = material->createInstance();
    }
}

Vector2 SpriteRender::center() const {
    return m_Center;
}

void SpriteRender::setCenter(const Vector2 &value) {
    m_Center    = value;
}

Texture *SpriteRender::texture() const {
    return m_Texture;
}

void SpriteRender::setTexture(Texture *texture) {
    m_Texture   = texture;
    if(m_pMaterial) {
        m_pMaterial->setTexture(OVERRIDE, m_Texture);
    }
}

Mesh *SpriteRender::mesh() const {
    return m_pMesh;
}

void SpriteRender::loadUserData(const VariantMap &data) {
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

VariantMap SpriteRender::saveUserData() const {
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
