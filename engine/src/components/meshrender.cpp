#include "components/meshrender.h"

#include "components/actor.h"
#include "components/transform.h"
#include "commandbuffer.h"
#include "pipelinecontext.h"

namespace {
    const char *gMesh = "Mesh";
}

/*!
    \class MeshRender
    \brief Draws a mesh for the 3D graphics.
    \inmodule Components

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
    if(m_mesh && !m_materials.empty() && layer & a->layers()) {
        Transform *t = a->transform();
        if(t) {
            buffer.setObjectId(a->uuid());
            buffer.setColor(Vector4(1.0f));

            const Matrix4 &transform = t->worldTransform();

            for(int i = 0; i < m_mesh->subMeshCount(); i++) {
                MaterialInstance *instance = (i < m_materials.size()) ? m_materials[i] : nullptr;
                if(instance) {
                    buffer.setMaterialId(instance->material()->uuid());
                    buffer.drawMesh(transform, m_mesh, i, layer, instance);
                }
            }
        }
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
        list<Material *> materials;
        for(int i = 0; i < m_mesh->subMeshCount(); i++) {
            materials.push_back(m_mesh->defaultMaterial(i));
        }

        setMaterialsList(materials);
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
    list<Material *> mats;

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
}
/*!
    \internal
*/
void MeshRender::setProperty(const char *name, const Variant &value) {
    Renderable::setProperty(name, value);
}
