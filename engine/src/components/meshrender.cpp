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
        m_meshInstance->decRef();
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

float MeshRender::blendShapeWeight(size_t index) const {
    if(index < m_blendShapeWeights.size()) {
        return m_blendShapeWeights[index];
    }
    return 0.0f;
}

void MeshRender::setBlendShapeWeight(size_t index, float weight) {
    if(index >= m_blendShapeWeights.size()) {
        m_blendShapeWeights.resize(index + 1, 0.0f);
    }
    m_blendShapeWeights[index] = weight;

    if(m_mesh && m_mesh->blendShapeCount() > index) {
        applyBlendShapeWeights();
    }
}

float MeshRender::blendShapeWeight(const TString &name) const {
    if(m_mesh) {
        const auto &shapes = m_mesh->blendShapes();
        for(size_t i = 0; i < shapes.size(); ++i) {
            if(shapes[i].name == name) {
                return blendShapeWeight(i);
            }
        }
    }
    return 0.0f;
}

void MeshRender::setBlendShapeWeight(const TString &name, float weight) {
    if(m_mesh) {
        const auto &shapes = m_mesh->blendShapes();
        for(size_t i = 0; i < shapes.size(); ++i) {
            if(shapes[i].name == name) {
                setBlendShapeWeight(i, weight);
                return;
            }
        }
    }
}

void MeshRender::clearBlendShapeWeights() {
    m_blendShapeWeights.clear();
    applyBlendShapeWeights();
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
    Applies current blend shape \a weights to the mesh vertices.
*/
void MeshRender::applyBlendShapeWeights() {
    if(m_mesh == nullptr) {
        return;
    }

    const auto &blendShapes = m_mesh->blendShapes();
    if(blendShapes.empty()) {
        return;
    }

    const Vector3Vector &baseVertices = m_mesh->vertices();
    const Vector3Vector &baseNormals = m_mesh->normals();
    const Vector3Vector &baseTangents = m_mesh->tangents();

    if(m_meshInstance == nullptr) {
        m_meshInstance = Engine::objectCreate<Mesh>(m_mesh->name());
        m_meshInstance->setVertices(baseVertices);
        m_meshInstance->setNormals(baseNormals);
        m_meshInstance->setTangents(baseTangents);
        m_meshInstance->setUv0(m_mesh->uv0());
        m_meshInstance->setUv1(m_mesh->uv1());
        m_meshInstance->setColors(m_mesh->colors());
        m_meshInstance->setBones(m_mesh->bones());
        m_meshInstance->setWeights(m_mesh->weights());
        m_meshInstance->setIndices(m_mesh->indices());
    }

    Vector3Vector &vertices = m_meshInstance->vertices();
    Vector3Vector &normals = m_meshInstance->normals();
    Vector3Vector &tangents = m_meshInstance->tangents();

    for(size_t shapeIndex = 0; shapeIndex < blendShapes.size(); ++shapeIndex) {
        const Mesh::BlendShape &shape = blendShapes[shapeIndex];
        float weight = 0.0f;
        if(shapeIndex < m_blendShapeWeights.size()) {
            weight = m_blendShapeWeights[shapeIndex];
        }
        if(weight == 0.0f) {
            continue;
        }

        const Mesh::BlendShapeFrame *activeFrame = nullptr;
        for(const auto &frame : shape.frames) {
            if(frame.weight != 0.0f) {
                activeFrame = &frame;
                break;
            }
        }

        if(!activeFrame) {
            continue;
        }

        for(size_t i = 0; i < activeFrame->indices.size(); ++i) {
            uint32_t index = activeFrame->indices[i];

            if(index >= baseVertices.size()) {
                continue;
            }

            vertices[index] = baseVertices[index] + activeFrame->vertices[i] * weight;
            if(i < activeFrame->normals.size()) {
                normals[index] = baseNormals[index] + activeFrame->normals[i] * weight;
            }
            if(i < activeFrame->tangents.size()) {
                tangents[index] = baseTangents[index] + activeFrame->tangents[i] * weight;
            }
        }
    }

    m_meshInstance->recalcBounds();
}
