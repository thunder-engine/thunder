#include "components/skinnedmeshrender.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/armature.h"

#include "commandbuffer.h"
#include "systems/resourcesystem.h"

#include "mesh.h"
#include "material.h"

namespace  {
const char *MESH = "Mesh";
const char *MATERIAL = "Material";
const char *ARMATURE = "Armature";
const char *MATRICES = "skinMatrices";
}

class SkinnedMeshRenderPrivate {
public:
    SkinnedMeshRenderPrivate() :
            m_pMesh(nullptr),
            m_pMaterial(nullptr),
            m_pArmature(nullptr) {

    }

    Mesh *m_pMesh;

    MaterialInstance *m_pMaterial;

    Armature *m_pArmature;
};
/*!
    \class SkinnedMeshRender
    \brief Draws an animated skeletal mesh for the 3D graphics.
    \inmodule Engine

    The SkinnedMeshRender component allows you to display 3D Skeletal Mesh to use in both 2D and 3D scenes.
*/

SkinnedMeshRender::SkinnedMeshRender() :
        p_ptr(new SkinnedMeshRenderPrivate) {

}

SkinnedMeshRender::~SkinnedMeshRender() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void SkinnedMeshRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(p_ptr->m_pMesh && layer & a->layers()) {
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
AABBox SkinnedMeshRender::bound() const {
    AABBox result;
    if(p_ptr->m_pMesh) {
        result = p_ptr->m_pMesh->bound();
    }
    if(p_ptr->m_pArmature) {
        result = p_ptr->m_pArmature->recalcBounds(result);
    }
    return result;
}
/*!
    Returns a Mesh assigned to this component.
*/
Mesh *SkinnedMeshRender::mesh() const {
    return p_ptr->m_pMesh;
}
/*!
    Assigns a new \a mesh to draw.
*/
void SkinnedMeshRender::setMesh(Mesh *mesh) {
    p_ptr->m_pMesh = mesh;
    if(p_ptr->m_pMesh) {
        Lod *lod = mesh->lod(0);
        if(lod) {
            setMaterial(lod->material());
        }
    }
}
/*!
    Returns an instantiated Material assigned to SkinnedMeshRender.
*/
Material *SkinnedMeshRender::material() const {
    if(p_ptr->m_pMaterial) {
        return p_ptr->m_pMaterial->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void SkinnedMeshRender::setMaterial(Material *material) {
    if(material) {
        if(p_ptr->m_pMaterial) {
            delete p_ptr->m_pMaterial;
        }
        p_ptr->m_pMaterial = material->createInstance(Material::Skinned);
        if(p_ptr->m_pArmature) {
            Texture *t = p_ptr->m_pArmature->texture();
            p_ptr->m_pMaterial->setTexture(MATRICES, t);
        }
    }
}
/*!
    Returns a Armature component for the attached skeleton.
*/
Armature *SkinnedMeshRender::armature() const {
    return p_ptr->m_pArmature;
}
/*!
    Attaches an \a armature skeleton.
*/
void SkinnedMeshRender::setArmature(Armature *armature) {
    p_ptr->m_pArmature = armature;
    if(p_ptr->m_pArmature) {
        connect(p_ptr->m_pArmature, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        if(p_ptr->m_pMaterial) {
            Texture *t = p_ptr->m_pArmature->texture();
            p_ptr->m_pMaterial->setTexture(MATRICES, t);
        }
    }
}
/*!
    \internal
*/
void SkinnedMeshRender::loadUserData(const VariantMap &data) {
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
    {
        auto it = data.find(ARMATURE);
        if(it != data.end()) {
            uint32_t uuid = uint32_t((*it).second.toInt());
            Object *object = Engine::findObject(uuid, Engine::findRoot(this));
            setArmature(dynamic_cast<Armature *>(object));
        }
    }
}
/*!
    \internal
*/
VariantMap SkinnedMeshRender::saveUserData() const {
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
    {
        if(p_ptr->m_pArmature) {
            result[ARMATURE] = int(p_ptr->m_pArmature->uuid());
        }
    }
    return result;
}

void SkinnedMeshRender::onReferenceDestroyed() {
    if(sender() == p_ptr->m_pArmature) {
        setArmature(nullptr);
    }
}

#ifdef NEXT_SHARED
#include "handles.h"

bool SkinnedMeshRender::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        AABBox aabb = bound();
        Handles::drawBox(aabb.center, Quaternion(), aabb.extent * 2.0f);
    }
    return false;
}
#endif
