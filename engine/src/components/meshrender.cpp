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
        m_dirtyMaterial(true) {

}
MeshRender::~MeshRender() {
    if(m_mesh) {
        m_mesh->decRef();
    }
}
/*!
    \internal
*/
Mesh *MeshRender::meshToDraw(int instance) {
    A_UNUSED(instance);
    return m_mesh;
}
/*!
    \internal
*/
MaterialInstance *MeshRender::materialInstance(int index) {
    if(m_dirtyMaterial && !m_materials.empty()) {
        MaterialInstance *inst = m_materials.front();
        if(inst) {
            inst->setTransform(transform());

            m_dirtyMaterial = false;
        }
    }

    return Renderable::materialInstance(index);
}
/*!
    \internal
*/
void MeshRender::setMaterialsList(const std::list<Material *> &materials) {
    Renderable::setMaterialsList(materials);

    m_dirtyMaterial = true;
}
/*!
    \internal
*/
AABBox MeshRender::localBound() const {
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
