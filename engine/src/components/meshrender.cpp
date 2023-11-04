#include "components/meshrender.h"

#include "components/actor.h"
#include "components/transform.h"
#include "commandbuffer.h"
#include "pipelinecontext.h"

#include "mesh.h"

namespace {
const char *gMesh = "Mesh";
}

/*!
    \class MeshRender
    \brief Draws a mesh for the 3D graphics.
    \inmodule Engine

    The MeshRender component allows you to display 3D Mesh to use in both 2D and 3D scenes.
*/

MeshRender::MeshRender() :
    m_mesh(nullptr) {

}
/*!
    \internal
*/
void MeshRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(m_mesh && !m_materials.empty() && layer & a->layers() && a->transform()) {
        buffer.setObjectId(a->uuid());
        buffer.setMaterialId(m_materials[0]->material()->uuid());
        buffer.setColor(Vector4(1.0f));

        buffer.drawMesh(a->transform()->worldTransform(), m_mesh, 0, layer, m_materials[0]);
    }
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
    m_mesh = mesh;
    if(m_mesh && m_materials.empty()) {
        setMaterial(m_mesh->material());
    }
}
/*!
    \internal
*/
void MeshRender::loadUserData(const VariantMap &data) {
    Renderable::loadUserData(data);

    auto it = data.find(gMesh);
    if(it != data.end()) {
        setMesh(Engine::loadResource<Mesh>((*it).second.toString()));
    }
}
/*!
    \internal
*/
VariantMap MeshRender::saveUserData() const {
    VariantMap result(Renderable::saveUserData());

    string ref = Engine::reference(mesh());
    if(!ref.empty()) {
        result[gMesh] = ref;
    }

    return result;
}
/*!
    \internal
*/
void MeshRender::composeComponent() {
    setMesh(PipelineContext::defaultCube());
    setMaterial(Engine::loadResource<Material>(".embedded/DefaultMesh.mtl"));
}
/*!
    \internal
*/
void MeshRender::setProperty(const char *name, const Variant &value) {
    Renderable::setProperty(name, value);
}
