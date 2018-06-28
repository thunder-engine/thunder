#include "components/spritemesh.h"
#include "components/actor.h"

#include "resources/material.h"
#include "resources/mesh.h"

#include "commandbuffer.h"

#define MATERIAL    "Material"
#define BASEMAP     "BaseMap"

SpriteMesh::SpriteMesh() {
    m_Center    = Vector2();
    m_Texture   = nullptr;
    m_Materials.push_back(nullptr);
    m_pMesh     = Engine::loadResource<Mesh>(".embedded/plane.fbx");
}

void SpriteMesh::draw(ICommandBuffer &buffer, int8_t layer) {
    Actor &a    = actor();
    if(layer & (ICommandBuffer::RAYCAST | ICommandBuffer::DEFAULT | ICommandBuffer::TRANSLUCENT | ICommandBuffer::SHADOWCAST | ICommandBuffer::UI)) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(ICommandBuffer::idToColor(a.uuid()));
        }

        buffer.drawMesh(a.worldTransform(), m_pMesh, 0, layer, m_Materials[0]);
        buffer.setColor(Vector4(1.0f));
    }
}

Vector2 SpriteMesh::center() const {
    return m_Center;
}

void SpriteMesh::setCenter(const Vector2 &value) {
    m_Center    = value;
}

Texture *SpriteMesh::texture() const {
    return m_Texture;
}

void SpriteMesh::setTexture(Texture *texture) {
    m_Texture   = texture;
    if(m_Materials[0]) {
        m_Materials[0]->setTexture("texture0", m_Texture);
    }
}

void SpriteMesh::loadUserData(const VariantMap &data) {
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

VariantMap SpriteMesh::saveUserData() const {
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
