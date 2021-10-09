#include "components/meshrender.h"

#include "components/actor.h"
#include "components/transform.h"
#include "commandbuffer.h"

#include "mesh.h"
#include "material.h"

namespace {
const char *MESH = "Mesh";
const char *MATERIAL = "Material";
}

class MeshRenderPrivate {
public:
    MeshRenderPrivate() :
            m_pMesh(nullptr),
            m_pMaterial(nullptr) {
    }

    Mesh *m_pMesh;

    MaterialInstance *m_pMaterial;
};
/*!
    \class MeshRender
    \brief Draws a mesh for the 3D graphics.
    \inmodule Engine

    The MeshRender component allows you to display 3D Mesh to use in both 2D and 3D scenes.
*/

MeshRender::MeshRender() :
        p_ptr(new MeshRenderPrivate) {

}

MeshRender::~MeshRender() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void MeshRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(p_ptr->m_pMesh && layer & a->layers() && a->transform()) {
        if(layer & CommandBuffer::RAYCAST) {
            buffer.setColor(CommandBuffer::idToColor(a->uuid()));
        }

        buffer.drawMesh(a->transform()->worldTransform(), p_ptr->m_pMesh, 0, layer, p_ptr->m_pMaterial);
        buffer.setColor(Vector4(1.0f));
    }
}
/*!
    \internal
*/
AABBox MeshRender::bound() const {
    Transform *t = actor()->transform();
    if(p_ptr->m_pMesh && t) {
        return p_ptr->m_pMesh->bound() * t->worldTransform();
    }
    return Renderable::bound();
}
/*!
    Returns a Mesh assigned to this component.
*/
Mesh *MeshRender::mesh() const {
    return p_ptr->m_pMesh;
}
/*!
    Assigns a new \a mesh to draw.
*/
void MeshRender::setMesh(Mesh *mesh) {
    p_ptr->m_pMesh = mesh;
    if(p_ptr->m_pMesh) {
        Lod *lod = mesh->lod(0);
        if(lod) {
            setMaterial(lod->material());
        }
    }
}
/*!
    Returns an instantiated Material assigned to MeshRender.
*/
Material *MeshRender::material() const {
    if(p_ptr->m_pMaterial) {
        return p_ptr->m_pMaterial->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void MeshRender::setMaterial(Material *material) {
    if(material) {
        if(p_ptr->m_pMaterial) {
            delete p_ptr->m_pMaterial;
        }
        p_ptr->m_pMaterial = material->createInstance();
    }
}
/*!
    \internal
*/
void MeshRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(MESH);
        if(it != data.end()) {
            setMesh(Engine::loadResource<Mesh>((*it).second.toString()));
        }
    }
    if(p_ptr->m_pMesh) {
        auto it = data.find(MATERIAL);
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
            result[MESH] = ref;
        }
    }
    {
        string ref = Engine::reference(material());
        if(!ref.empty()) {
            result[MATERIAL] = ref;
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
