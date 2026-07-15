#include "components/meshrender.h"

#include "resources/material.h"

#include "pipelinecontext.h"
#include "gizmos.h"

/*!
    \class MeshRender
    \brief Draws a mesh for the 3D graphics.
    \inmodule Components

    The MeshRender component allows you to display 3D Mesh to use in both 2D and 3D scenes.
*/

MeshRender::MeshRender() :
        m_mesh(nullptr),
        m_meshInstance(nullptr) {

}
MeshRender::~MeshRender() {
    if(m_mesh) {
        m_mesh->decRef();
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
    return m_mesh;
}
/*!
    \internal
*/
AABBox MeshRender::localBound() {
    if(m_mesh) {
        return m_mesh->bound();
    }
    return Renderable::localBound();
}
/*!
    Returns a Mesh assigned to this component.
*/
Mesh *MeshRender::mesh() const {
    return m_mesh;
}
/*!
    Assigns a new \a mesh to draw.
*/
void MeshRender::setMesh(Mesh *mesh) {
    if(m_mesh != mesh) {
        if(m_mesh) {
            m_mesh->decRef();
        }

        if(m_meshInstance) {
            delete m_meshInstance;
            m_meshInstance = nullptr;
        }

        m_mesh = mesh;
        m_blendShapeWeights.clear();
        if(m_mesh) {
            m_mesh->incRef();

            if(m_materials.empty()) {
                std::list<Material *> materials;
                for(int i = 0; i < m_mesh->subMeshCount(); i++) {
                    materials.push_back(m_mesh->defaultMaterial(i));
                }

                setMaterialsList(materials);
            }
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

    if(m_mesh && m_mesh->blendShapeCount() > index) {
        if(ensureMeshInstance()) {
            applyBlendShapeWeights(*m_mesh, *m_meshInstance, m_blendShapeWeights);
        }
    }
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
Mesh *MeshRender::ensureMeshInstance() {
    if(m_mesh == nullptr) {
        return nullptr;
    }

    if(m_meshInstance == nullptr) {
        m_meshInstance = Engine::objectCreate<Mesh>();
        m_meshInstance->setIndices(m_mesh->indices());
        m_meshInstance->setVertices(m_mesh->vertices());
        m_meshInstance->setNormals(m_mesh->normals());
        m_meshInstance->setTangents(m_mesh->tangents());
        m_meshInstance->setUv0(m_mesh->uv0());
        m_meshInstance->setUv1(m_mesh->uv1());
        m_meshInstance->setColors(m_mesh->colors());
        m_meshInstance->setBones(m_mesh->bones());
        m_meshInstance->setWeights(m_mesh->weights());
    }

    return m_meshInstance;
}
