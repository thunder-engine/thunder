#include "components/skinnedmeshrender.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/armature.h"

#include "resources/material.h"

namespace  {
    const char *gMatrices("skinMatrices");
}

/*!
    \class SkinnedMeshRender
    \brief Draws an animated skeletal mesh for the 3D graphics.
    \inmodule Components

    The SkinnedMeshRender component allows you to display 3D Skeletal Mesh to use in both 2D and 3D scenes.
*/

SkinnedMeshRender::SkinnedMeshRender() :
        m_armature(nullptr) {

    m_bounds.radius = 0.0f;
    m_surfaceType = Material::Skinned;
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
    Creates a new instance of \a material and assigns it.
*/
void SkinnedMeshRender::setMaterial(Material *material) {
    Renderable::setMaterial(material);

    for(auto it : m_materials) {
        if(it) {
            if(m_armature) {
                it->setTexture(gMatrices, m_armature->texture());
            }
            it->setTransform(transform());
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
    if(m_armature != armature) {
        if(m_armature) {
            disconnect(m_armature, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        }

        m_armature = armature;
        if(m_armature) {
            connect(m_armature, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            for(auto it : m_materials) {
                if(it) {
                    it->setTexture(gMatrices, m_armature->texture());
                }
            }

            m_armature->update();
        }
    }
}
/*!
    \internal
*/
void SkinnedMeshRender::setMaterialsList(const std::list<Material *> &materials) {
    Renderable::setMaterialsList(materials);

    for(auto it : m_materials) {
        if(it) {
            if(m_armature) {
                it->setTexture(gMatrices, m_armature->texture());
            }
            it->setTransform(transform());
        }
    }
}
/*!
    \internal
*/
void SkinnedMeshRender::onReferenceDestroyed() {
    MeshRender::onReferenceDestroyed();

    if(sender() == m_armature) {
        setArmature(nullptr);
    }
}
