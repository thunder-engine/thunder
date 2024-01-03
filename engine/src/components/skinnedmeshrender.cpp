#include "components/skinnedmeshrender.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/armature.h"

#include "resources/mesh.h"
#include "resources/material.h"

#include "systems/resourcesystem.h"

#include "commandbuffer.h"
#include "gizmos.h"

namespace  {
    const char *gMesh = "Mesh";
    const char *gArmature= "Armature";
    const char *gMatrices = "skinMatrices";
}

/*!
    \class SkinnedMeshRender
    \brief Draws an animated skeletal mesh for the 3D graphics.
    \inmodule Components

    The SkinnedMeshRender component allows you to display 3D Skeletal Mesh to use in both 2D and 3D scenes.
*/

SkinnedMeshRender::SkinnedMeshRender() :
        m_mesh(nullptr),
        m_armature(nullptr) {

    m_bounds.radius = 0.0f;
    m_surfaceType = Material::Skinned;
}
/*!
    \internal
*/
void SkinnedMeshRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(m_mesh && !m_materials.empty() && layer & a->layers()) {
        buffer.setObjectId(a->uuid());
        buffer.setColor(Vector4(1.0f));

        for(int i = 0; i < m_mesh->subMeshCount(); i++) {
            MaterialInstance *instance = (i < m_materials.size()) ? m_materials[i] : nullptr;
            if(instance) {
                buffer.setMaterialId(instance->material()->uuid());
                buffer.drawMesh(a->transform()->worldTransform(), m_mesh, i, layer, instance);
            }
        }
    }
}
/*!
    \internal
*/
AABBox SkinnedMeshRender::localBound() const {
    return m_bounds;
}
/*!
    Returns the center of the local bounding box.
*/
Vector3 SkinnedMeshRender::boundsCenter() const {
    return m_bounds.center;
}
/*!
    Sets the \a center of the local bounding box.
*/
void SkinnedMeshRender::setBoundsCenter(Vector3 center) {
    m_bounds.center = center;
}
/*!
    Returns the extent of the local bounding box.
*/
Vector3 SkinnedMeshRender::boundsExtent() const {
    return m_bounds.extent;
}
/*!
    Sets the \a extent of the local bounding box.
*/
void SkinnedMeshRender::setBoundsExtent(Vector3 extent) {
    m_bounds.extent = extent;
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
        if(m_materials.empty()) {
            list<Material *> materials;
            for(int i = 0; i < m_mesh->subMeshCount(); i++) {
                materials.push_back(m_mesh->defaultMaterial(i));
            }

            setMaterialsList(materials);
        }

        if(!m_bounds.isValid()) {
            m_bounds = m_mesh->bound();
        }
    }
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void SkinnedMeshRender::setMaterial(Material *material) {
    Renderable::setMaterial(material);

    if(!m_materials.empty() && m_armature) {
        m_materials[0]->setTexture(gMatrices, m_armature->texture());
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
        if(!m_materials.empty()) {
            m_materials[0]->setTexture(gMatrices, m_armature->texture());
        }
    }
}
/*!
    Returns a list of assigned materials.
*/
VariantList SkinnedMeshRender::materials() const {
    VariantList result;

    for(auto it : m_materials) {
        result.push_back(Variant::fromValue<Material *>(it ? it->material() : nullptr));
    }

    return result;
}
/*!
    Assigns an array of the \a materials to the mesh.
*/
void SkinnedMeshRender::setMaterials(VariantList materials) {
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
void SkinnedMeshRender::loadUserData(const VariantMap &data) {
    Renderable::loadUserData(data);

    auto it = data.find(gMesh);
    if(it != data.end()) {
        setMesh(Engine::loadResource<Mesh>((*it).second.toString()));
    }

    it = data.find(gArmature);
    if(it != data.end()) {
        uint32_t uuid = uint32_t((*it).second.toInt());
        Object *object = Engine::findObject(uuid, Engine::findRoot(this));
        setArmature(dynamic_cast<Armature *>(object));
    }
}
/*!
    \internal
*/
VariantMap SkinnedMeshRender::saveUserData() const {
    VariantMap result(Renderable::saveUserData());

    string ref = Engine::reference(mesh());
    if(!ref.empty()) {
        result[gMesh] = ref;
    }

    if(m_armature) {
        result[gArmature] = int(m_armature->uuid());
    }

    return result;
}
/*!
    \internal
*/
void SkinnedMeshRender::onReferenceDestroyed() {
    if(sender() == m_armature) {
        setArmature(nullptr);
    }
}
/*!
    \internal
*/
void SkinnedMeshRender::drawGizmosSelected() {
    AABBox aabb = bound();
    Gizmos::drawWireBox(aabb.center, aabb.extent * 2.0f, Vector4(1.0f));
}
