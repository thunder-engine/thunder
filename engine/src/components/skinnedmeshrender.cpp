#include "components/skinnedmeshrender.h"

#include "components/armature.h"

#include "resources/material.h"

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
AABBox SkinnedMeshRender::localBound() {
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
void SkinnedMeshRender::setBoundsCenter(const Vector3 &center) {
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
void SkinnedMeshRender::setBoundsExtent(const Vector3 &extent) {
    m_bounds.extent = extent;
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

            for(auto it : m_materials) {
                m_armature->removeInstance(it);
            }
        }

        m_armature = armature;
        if(m_armature) {
            connect(m_armature, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));

            for(auto it : m_materials) {
                m_armature->addInstance(it);
            }

            m_armature->update();
        }
    }
}
/*!
    \internal
*/
void SkinnedMeshRender::setMaterialsList(const std::list<Material *> &materials) {
    if(m_armature) {
        for(auto it : m_materials) {
            m_armature->removeInstance(it);
        }
    }

    MeshRender::setMaterialsList(materials);

    for(auto it : m_materials) {
        if(it) {
            if(m_armature) {
                m_armature->addInstance(it);
            }
        }
    }
}
/*!
    \internal
*/
void SkinnedMeshRender::onReferenceDestroyed() {
    MeshRender::onReferenceDestroyed();

    if(sender() == m_armature) {
        m_armature = nullptr;
    }
}
