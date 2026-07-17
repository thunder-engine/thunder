#include "components/meshrender.h"

#include "resources/material.h"

#include "systems/resourcesystem.h"

#include "pipelinecontext.h"
#include "gizmos.h"

/*!
    \class MeshRender
    \brief Draws a mesh for the 3D graphics.
    \inmodule Components

    The MeshRender component allows you to display 3D Mesh to use in both 2D and 3D scenes.
*/

MeshRender::MeshRender() :
        m_baseMesh(nullptr),
        m_meshInstance(nullptr) {

    for(auto &it : m_lods) {
        it.first = nullptr;
        it.second = false;
    }
}
MeshRender::~MeshRender() {
    for(auto &it : m_lods) {
        if(it.first) {
            it.first->decRef();
            it.first = nullptr;
        }
        it.second = false;
    }

    if(m_meshInstance) {
        delete m_meshInstance;
        m_meshInstance = nullptr;
    }
}
/*!
    \internal
*/
Mesh *MeshRender::meshToDraw() {
    if(m_meshInstance) {
        return m_meshInstance;
    }

    Mesh *lastMesh = m_baseMesh;
    for(int32_t i = 2; i >= static_cast<int32_t>(m_lod); --i) {
        if(m_lods[i].first) {
            Resource::State state = m_lods[i].first->state();
            if(state >= Resource::ToBeUpdated && state <= Resource::Ready) {
                lastMesh = m_lods[i].first;
            }
        } else if(i == m_lod && !m_lods[i].second) {
            ResourceSystem *system = Engine::resourceSystem();
            m_lods[i].first = dynamic_cast<Mesh *>(system->loadResourceAsync(m_meshRef, i));
            m_lods[i].second = true;
            if(m_lods[i].first) {
                m_lods[i].first->incRef();
                lastMesh = m_lods[i].first;
            }
        }
    }

    return lastMesh;
}
/*!
    \internal
*/
AABBox MeshRender::localBound() {
    if(m_baseMesh) {
        return m_baseMesh->bound();
    }
    return Renderable::localBound();
}
/*!
    Returns a Mesh assigned to this component.
*/
Mesh *MeshRender::mesh() const {
    return m_baseMesh;
}
/*!
    Assigns a new \a mesh to draw.
*/
void MeshRender::setMesh(Mesh *mesh) {
    if(m_baseMesh != mesh) {
        for(auto &it : m_lods) {
            if(it.first) {
                it.first->decRef();
                it.first = nullptr;
            }
            it.second = false;
        }

        if(m_meshInstance) {
            delete m_meshInstance;
            m_meshInstance = nullptr;
        }

        m_baseMesh = mesh;
        if(m_baseMesh) {
            m_baseMesh->incRef();

            m_meshRef = Engine::reference(m_baseMesh);

            updateBlendShapes();

            if(m_materials.empty()) {
                std::list<Material *> materials;
                for(int i = 0; i < m_baseMesh->subMeshCount(); i++) {
                    materials.push_back(m_baseMesh->defaultMaterial(i));
                }

                setMaterialsList(materials);
            }
        } else {
            m_meshRef.clear();
        }
    }
}
/*!
    Returns weight of a blend shape  with specified \a index.
*/
float MeshRender::blendShapeWeight(size_t index) const {
    if(index < m_blendShapeWeights.size()) {
        return m_blendShapeWeights[index];
    }
    return 0.0f;
}
/*!
    Sets the \a weight of a blend shape with specified \a index for this renderer.
*/
void MeshRender::setBlendShapeWeight(size_t index, float weight) {
    if(index >= m_blendShapeWeights.size()) {
        m_blendShapeWeights.resize(index + 1, 0.0f);
    }
    m_blendShapeWeights[index] = weight;

    updateBlendShapes();
}
/*!
    Returns a list of assigned materials.
*/
VariantList MeshRender::materials() const {
    VariantList result;

    for(auto it : m_materials) {
        result.push_back(Variant::fromValue<Material *>(it ? it->material() : nullptr));
    }

    return result;
}
/*!
    Assigns an array of the \a materials to the mesh.
*/
void MeshRender::setMaterials(VariantList materials) {
    std::list<Material *> mats;

    for(auto &it : materials) {
        Object *object = *reinterpret_cast<Object **>(it.data());
        Material *material = dynamic_cast<Material *>(object);

        mats.push_back(material);
    }

    setMaterialsList(mats);
}
/*!
    \internal
*/
void MeshRender::drawGizmosSelected() {
    AABBox aabb = bound();
    Gizmos::drawWireBox(aabb.center, aabb.extent * 2.0f, Vector4(1.0f));
}
/*!
    \internal
*/
void MeshRender::composeComponent() {
    setMesh(PipelineContext::defaultCube());
}
/*!
    \internal
*/
bool MeshRender::ensureMeshInstance() {
    if(m_lods[m_lod].first == nullptr) {
        return false;
    }

    if(m_meshInstance == nullptr) {
        m_meshInstance = Engine::objectCreate<Mesh>();
        m_meshInstance->setIndices(m_lods[m_lod].first->indices());
        m_meshInstance->setVertices(m_lods[m_lod].first->vertices());
        m_meshInstance->setNormals(m_lods[m_lod].first->normals());
        m_meshInstance->setTangents(m_lods[m_lod].first->tangents());
        m_meshInstance->setUv0(m_lods[m_lod].first->uv0());
        m_meshInstance->setUv1(m_lods[m_lod].first->uv1());
        m_meshInstance->setColors(m_lods[m_lod].first->colors());
        m_meshInstance->setBones(m_lods[m_lod].first->bones());
        m_meshInstance->setWeights(m_lods[m_lod].first->weights());
    }

    return true;
}
/*!
    \internal
*/
void MeshRender::updateBlendShapes() {
    if(m_lod < 3 && m_lods[m_lod].first && m_lods[m_lod].first->blendShapeCount() > 0) {
        if(ensureMeshInstance()) {
            applyBlendShapeWeights(*(m_lods[m_lod].first), *m_meshInstance, m_blendShapeWeights);
        }
    }
}
