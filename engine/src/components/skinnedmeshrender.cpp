#include "components/skinnedmeshrender.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/armature.h"

#include "commandbuffer.h"
#include "systems/resourcesystem.h"

#include "mesh.h"
#include "material.h"

namespace  {
const char *gMesh = "Mesh";
const char *gMaterial = "Material";
const char *gArmature= "Armature";
const char *gMatrices = "skinMatrices";
}

/*!
    \class SkinnedMeshRender
    \brief Draws an animated skeletal mesh for the 3D graphics.
    \inmodule Engine

    The SkinnedMeshRender component allows you to display 3D Skeletal Mesh to use in both 2D and 3D scenes.
*/

SkinnedMeshRender::SkinnedMeshRender() :
    m_mesh(nullptr),
    m_material(nullptr),
    m_armature(nullptr) {

}
/*!
    \internal
*/
void SkinnedMeshRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(m_mesh && layer & a->layers()) {
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
AABBox SkinnedMeshRender::localBound() const {
    AABBox result;
    if(m_mesh) {
        result = m_mesh->bound();
    }
    if(m_armature) {
        result = m_armature->recalcBounds(result);
    }
    return result;
}
/*!
    Returns a Mesh assigned to this component.
*/
Mesh *SkinnedMeshRender::mesh() const {
    return m_mesh;
}
/*!
    Assigns a new \a mesh to draw.
*/
void SkinnedMeshRender::setMesh(Mesh *mesh) {
    m_mesh = mesh;
    if(m_mesh) {
        setMaterial(m_mesh->material());
    }
}
/*!
    Returns an instantiated Material assigned to SkinnedMeshRender.
*/
Material *SkinnedMeshRender::material() const {
    if(m_material) {
        return m_material->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void SkinnedMeshRender::setMaterial(Material *material) {
    if(material) {
        if(m_material) {
            delete m_material;
        }
        m_material = material->createInstance(Material::Skinned);
        if(m_armature) {
            Texture *t = m_armature->texture();
            m_material->setTexture(gMatrices, t);
        }
    }
}
/*!
    Returns a Armature component for the attached skeleton.
*/
Armature *SkinnedMeshRender::armature() const {
    return m_armature;
}
/*!
    Attaches an \a armature skeleton.
*/
void SkinnedMeshRender::setArmature(Armature *armature) {
    m_armature = armature;
    if(m_armature) {
        connect(m_armature, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        if(m_material) {
            Texture *t = m_armature->texture();
            m_material->setTexture(gMatrices, t);
        }
    }
}
/*!
    \internal
*/
void SkinnedMeshRender::loadUserData(const VariantMap &data) {
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
    {
        auto it = data.find(gArmature);
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
            result[gMesh] = ref;
        }
    }
    {
        string ref = Engine::reference(material());
        if(!ref.empty()) {
            result[gMaterial] = ref;
        }
    }
    {
        if(m_armature) {
            result[gArmature] = int(m_armature->uuid());
        }
    }
    return result;
}

void SkinnedMeshRender::onReferenceDestroyed() {
    if(sender() == m_armature) {
        setArmature(nullptr);
    }
}

#ifdef SHARED_DEFINE
#include "viewport/handles.h"

bool SkinnedMeshRender::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        AABBox aabb = bound();
        Handles::drawBox(aabb.center, Quaternion(), aabb.extent * 2.0f);
    }
    return false;
}
#endif
