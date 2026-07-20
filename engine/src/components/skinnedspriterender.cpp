#include "components/skinnedspriterender.h"

#include "resources/material.h"

/*!
    \class SkinnedSpriteRender
    \brief Draws an animated skeletal sprite for the 2D graphics.
    \inmodule Components

    The SkinnedSpriteRender component allows you to display 2D Skeletal Sprite to use in both 2D and 3D scenes.
*/

SkinnedSpriteRender::SkinnedSpriteRender() :
        m_armature(nullptr) {

    m_bounds.radius = 0.0f;
    m_surfaceType = Material::Skinned;
}
/*!
    \internal
*/
AABBox SkinnedSpriteRender::localBound() {
    return m_bounds;
}
/*!
    Returns the center of the local bounding box.
*/
Vector3 SkinnedSpriteRender::boundsCenter() const {
    return m_bounds.center;
}
/*!
    Sets the \a center of the local bounding box.
*/
void SkinnedSpriteRender::setBoundsCenter(const Vector3 &center) {
    m_bounds.center = center;
}
/*!
    Returns the extent of the local bounding box.
*/
Vector3 SkinnedSpriteRender::boundsExtent() const {
    return m_bounds.extent;
}
/*!
    Sets the \a extent of the local bounding box.
*/
void SkinnedSpriteRender::setBoundsExtent(const Vector3 &extent) {
    m_bounds.extent = extent;
}
/*!
    Returns a Armature component for the attached skeleton.
*/
Armature *SkinnedSpriteRender::armature() const {
    return m_armature;
}
/*!
    Attaches an \a armature skeleton.
*/
void SkinnedSpriteRender::setArmature(Armature *armature) {
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
void SkinnedSpriteRender::setMaterialsList(const std::list<Material *> &materials) {
    if(m_armature) {
        for(auto it : m_materials) {
            m_armature->removeInstance(it);
        }
    }

    SpriteRender::setMaterialsList(materials);

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
void SkinnedSpriteRender::onReferenceDestroyed() {
    SpriteRender::onReferenceDestroyed();

    if(sender() == m_armature) {
        m_armature = nullptr;
    }
}
