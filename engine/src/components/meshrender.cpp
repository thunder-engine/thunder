#include "components/meshrender.h"

#include "components/actor.h"
#include "components/transform.h"
#include "commandbuffer.h"

#define MESH "Mesh"
#define MATERAIL "Material"

MeshRender::MeshRender() :
        m_pMesh(nullptr),
        m_pMaterial(nullptr) {
}

void MeshRender::draw(ICommandBuffer &buffer, uint32_t layer) {
    Actor *a    = actor();
    if(m_pMesh && layer & a->layers()) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(ICommandBuffer::idToColor(a->uuid()));
        }

        buffer.drawMesh(a->transform()->worldTransform(), m_pMesh, 0, layer, m_pMaterial);
        buffer.setColor(Vector4(1.0f));
    }
}

AABBox MeshRender::bound() const {
    if(m_pMesh) {
        return m_pMesh->bound();
    }
    return Renderable::bound();
}

Mesh *MeshRender::mesh() const {
    return m_pMesh;
}

void MeshRender::setMesh(Mesh *mesh) {
    m_pMesh    = mesh;
    if(m_pMesh) {
        delete m_pMaterial;
        for(uint32_t l = 0; l < mesh->lodsCount(0); l++) {
            Material *m = mesh->material(0, l);
            m_pMaterial = (m) ? m->createInstance() : nullptr;
        }
    }
}

Material *MeshRender::material() const {
    if(m_pMaterial) {
        return m_pMaterial->material();
    }
    return nullptr;
}

void MeshRender::setMaterial(Material *material) {
    if(material) {
        if(m_pMaterial) {
            delete m_pMaterial;
        }
        m_pMaterial = material->createInstance();
    }
}

void MeshRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(MESH);
        if(it != data.end()) {
            setMesh(Engine::loadResource<Mesh>((*it).second.toString()));
        }
    }
    if(m_pMesh) {
        auto it = data.find(MATERAIL);
        if(it != data.end()) {
            setMaterial(Engine::loadResource<Material>((*it).second.toString()));
        }
    }
}

VariantMap MeshRender::saveUserData() const {
    VariantMap result = Component::saveUserData();
    {
        string ref = Engine::reference(mesh());
        if(!ref.empty()) {
            result[MESH] = ref;
        }
    }
    {
        string ref = Engine::reference(material());
        if(!ref.empty()) {
            result[MATERAIL] = ref;
        }
    }
    return result;
}
