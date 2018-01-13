#include "components/staticmesh.h"

#include "resources/material.h"

#define MESH "Mesh"
#define MATERAILS "Materials"

StaticMesh::StaticMesh() :
        m_pMesh(nullptr) {
}

void StaticMesh::update() {
}

Mesh *StaticMesh::mesh() const {
    return m_pMesh;
}

void StaticMesh::setMesh(Mesh *mesh) {
    m_pMesh    = mesh;
    if(m_pMesh) {
        m_Materials.clear();
        for(auto &surface : mesh->m_Surfaces) {
            for(auto &lod : surface.lods) {
                m_Materials.push_back(lod.material);
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
        return m_Materials[index];
    }
    return nullptr;
}

void StaticMesh::setMaterial(uint32_t index, Material *mat) {
    if(index < m_Materials.size()) {
        m_Materials[index]  = mat;
    }
}

uint32_t StaticMesh::materialCount() const {
    return m_Materials.size();
}

void StaticMesh::loadUserData(const AVariantMap &data) {
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
            AVariantList list = (*it).second.value<AVariantList>();
            uint32_t i  = 0;
            for(auto it : list) {
                setMaterial(i,  Engine::loadResource<Material>(it.toString()));
                i++;
            }
        }
    }
}

AVariantMap StaticMesh::saveUserData() const {
    AVariantMap result    = Component::saveUserData();
    {
        Mesh *m     = mesh();
        string ref  = Engine::reference(m);
        if(!ref.empty()) {
            result[MESH]    = ref;
        }
    }
    {
        AVariantList list;
        for(Material *it : materials()) {
            list.push_back(Engine::reference(it));
        }
        result[MATERAILS]   = list;
    }
    return result;
}
