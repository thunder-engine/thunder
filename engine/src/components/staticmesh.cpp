#include "components/staticmesh.h"
#include "components/actor.h"

#include "resources/material.h"

#include "commandbuffer.h"

#define MESH "Mesh"
#define MATERAILS "Materials"

StaticMesh::StaticMesh() :
        m_pMesh(nullptr) {
}

void StaticMesh::draw(ICommandBuffer &buffer, int8_t layer) {
    Actor &a    = actor();
    if(m_pMesh && layer & (ICommandBuffer::RAYCAST | ICommandBuffer::DEFAULT | ICommandBuffer::TRANSLUCENT | ICommandBuffer::SHADOWCAST)) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(ICommandBuffer::idToColor(a.uuid()));
        }

        for(uint32_t s = 0; s < m_pMesh->surfacesCount(); s++) {
            MaterialInstance *material   = (s < m_Materials.size()) ? m_Materials[s] : nullptr;
            buffer.drawMesh(a.worldTransform(), m_pMesh, s, layer, material);
        }
        buffer.setColor(Vector4(1.0f));
    }
}

Mesh *StaticMesh::mesh() const {
    return m_pMesh;
}

void StaticMesh::setMesh(Mesh *mesh) {
    m_pMesh    = mesh;
    if(m_pMesh) {
        for(auto it : m_Materials) {
            delete it;
        }
        m_Materials.clear();
        for(uint32_t s = 0; s < mesh->surfacesCount(); s++) {
            for(uint32_t l = 0; l < mesh->lodsCount(s); l++) {
                m_Materials.push_back(mesh->material(s, l)->createInstance());
            }
        }
    }
}

MaterialArray StaticMesh::materials() const {
    return m_Materials;
}

void StaticMesh::setMaterials(const MaterialArray &mat) {
    m_Materials = mat;
}

Material *StaticMesh::material(uint32_t index) const {
    if(index < m_Materials.size()) {
        return m_Materials[index]->material();
    }
    return nullptr;
}

void StaticMesh::setMaterial(uint32_t index, Material *mat) {
    if(index < m_Materials.size()) {
        if(m_Materials[index]) {
            delete m_Materials[index];
        }
        m_Materials[index]  = mat->createInstance();
    }
}

uint32_t StaticMesh::materialCount() const {
    return m_Materials.size();
}

void StaticMesh::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(MESH);
        if(it != data.end()) {
            setMesh(Engine::loadResource<Mesh>((*it).second.toString()));
        }
    }
    if(m_pMesh) {
        auto it = data.find(MATERAILS);
        if(it != data.end()) {
            VariantList list    = (*it).second.value<VariantList>();
            uint32_t i  = 0;
            for(auto it : list) {
                setMaterial(i,  Engine::loadResource<Material>(it.toString()));
                i++;
            }
        }
    }
}

VariantMap StaticMesh::saveUserData() const {
    VariantMap result   = Component::saveUserData();
    {
        Mesh *m     = mesh();
        string ref  = Engine::reference(m);
        if(!ref.empty()) {
            result[MESH]    = ref;
        }
    }
    {
        VariantList list;
        for(MaterialInstance *it : materials()) {
            list.push_back(Engine::reference(it->material()));
        }
        result[MATERAILS]   = list;
    }
    return result;
}
