#include "components/meshrender.h"

#include "components/actor.h"
#include "components/transform.h"
#include "commandbuffer.h"

#include "mesh.h"
#include "material.h"

namespace {
const char *gMesh = "Mesh";
const char *gMaterial = "Material";
}

/*!
    \class MeshRender
    \brief Draws a mesh for the 3D graphics.
    \inmodule Engine

    The MeshRender component allows you to display 3D Mesh to use in both 2D and 3D scenes.
*/

MeshRender::MeshRender() :
    m_mesh(nullptr),
    m_material(nullptr) {

}
/*!
    \internal
*/
void MeshRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(m_mesh && layer & a->layers() && a->transform()) {
        if(layer & CommandBuffer::RAYCAST) {
            buffer.setColor(CommandBuffer::idToColor(a->uuid()));
        }

        buffer.drawMesh(a->transform()->worldTransform(), m_mesh, 0, layer, m_material);
        buffer.setColor(Vector4(1.0f));
    }
}
/*!
    \internal
*/
AABBox MeshRender::bound() const {
    Transform *t = actor()->transform();
    if(m_mesh && t) {
        return m_mesh->bound() * t->worldTransform();
    }
    return Renderable::bound();
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
    if(m_mesh) {
        setMaterial(m_mesh->material());
    }
}
/*!
    Returns an instantiated Material assigned to MeshRender.
*/
Material *MeshRender::material() const {
    if(m_material) {
        return m_material->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void MeshRender::setMaterial(Material *material) {
    if(material) {
        if(m_material) {
            delete m_material;
        }
        m_material = material->createInstance();
    }
}
/*!
    \internal
*/
void MeshRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(gMesh);
        if(it != data.end()) {
            setMesh(Engine::loadResource<Mesh>((*it).second.toString()));
        }
    }
    if(m_mesh) {
        auto it = data.find(gMaterial);
        if(it != data.end()) {
            setMaterial(Engine::loadResource<Material>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap MeshRender::saveUserData() const {
    VariantMap result = Component::saveUserData();
    {
        string ref = Engine::reference(mesh());
        if(!ref.empty()) {
            result[gMesh] = ref;
        }
    }
    {
        string ref = Engine::reference(material());
        if(!ref.empty()) {
            result[gMaterial] = ref;
        }
    }
    return result;
}
/*!
    \internal
*/
void MeshRender::composeComponent() {
    setMesh(Engine::loadResource<Mesh>(".embedded/cube.fbx/Box001"));
    setMaterial(Engine::loadResource<Material>(".embedded/DefaultMesh.mtl"));
}
