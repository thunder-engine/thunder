#include "components/basemesh.h"

#define MESH "Mesh"
#define MATERAILS "Materials"

BaseMesh::BaseMesh() :
        m_pMesh(nullptr) {
}

Mesh *BaseMesh::mesh() const {
    return m_pMesh;
}

void BaseMesh::setMesh(Mesh *mesh) {
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

MaterialArray BaseMesh::materials() const {
    return m_Materials;
}

void BaseMesh::setMaterials(const MaterialArray &material) {
    m_Materials = material;
}

Material *BaseMesh::material(uint32_t index) const {
    if(index < m_Materials.size()) {
        return m_Materials[index]->material();
    }
    return nullptr;
}

void BaseMesh::setMaterial(Material *material, uint32_t index) {
    if(index < m_Materials.size()) {
        if(m_Materials[index]) {
            delete m_Materials[index];
        }
        m_Materials[index]  = material->createInstance();
    }
}

uint32_t BaseMesh::materialCount() const {
    return m_Materials.size();
}

void BaseMesh::loadUserData(const VariantMap &data) {
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
                setMaterial(Engine::loadResource<Material>(it.toString()), i);
                i++;
            }
        }
    }
}

VariantMap BaseMesh::saveUserData() const {
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
