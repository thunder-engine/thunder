#include "components/spriterender.h"

#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/sprite.h"

#include "pipelinecontext.h"

#include <math.h>

namespace  {
    const char *gColor = "mainColor";
    const char *gTexture = "mainTexture";
    const char *gDefaultSprite = ".embedded/DefaultSprite.shader";
}

/*!
    \class SpriteRender
    \brief Draws a sprite for the 2D graphics.
    \inmodule Components

    The SpriteRender component allows you to display images as sprites to use in both 2D and 3D scenes.
*/

/*!
    \enum SpriteRender::DrawMode

    \value Simple \c The SpriteRender will draw sprite as is.
    \value Sliced \c The SpriteRender will respect 9 patch slicing rules for the size property.
    \value Tiled \c The SpriteRender will duplicate sprites to fill the size property rectangle.
*/

SpriteRender::SpriteRender() :
        m_color(1.0f),
        m_size(1.0f),
        m_sheet(nullptr),
        m_texture(nullptr),
        m_mesh(PipelineContext::defaultPlane()),
        m_customMesh(Engine::objectCreate<Mesh>()),
        m_drawMode(Simple),
        m_useCustom(false),
        m_dirtyMesh(true),
        m_dirtyMaterial(true) {

}

SpriteRender::~SpriteRender() {
    if(m_sheet) {
        m_sheet->unsubscribe(this);
    }

    delete m_customMesh;
    m_customMesh = nullptr;
}
/*!
    \internal
*/
Mesh *SpriteRender::meshToDraw(int instance) {
    if(m_dirtyMesh) {
        m_useCustom = false;
        if(m_sheet) {
            int hash = Mathf::hashString(m_item);

            Mesh *mesh = m_sheet->composeMesh(m_customMesh, hash, static_cast<Sprite::Mode>(m_drawMode), m_size);
            if(mesh != m_customMesh) {
                if(mesh) {
                    m_mesh = mesh;
                    if(m_drawMode == Sprite::Simple) {
                        m_size = mesh->bound().extent * 2.0f;
                    }
                } else {
                    m_mesh = PipelineContext::defaultPlane();
                }
            } else {
                m_useCustom = true;
            }
        } else {
            m_mesh = PipelineContext::defaultPlane();
        }

        m_dirtyMesh = false;
    }

    return (m_useCustom) ? m_customMesh : m_mesh;
}
/*!
    \internal
*/
MaterialInstance *SpriteRender::materialInstance(int index) {
    if(m_dirtyMaterial && !m_materials.empty()) {
        MaterialInstance *inst = m_materials.front();
        if(inst) {
            inst->setTexture(gTexture, texture());
            inst->setVector4(gColor, &m_color);
            inst->setTransform(transform());
            inst->setPriority(m_priority);

            m_dirtyMaterial = false;
        }
    }

    return Renderable::materialInstance(index);
}
/*!
    \internal
*/
AABBox SpriteRender::localBound() const {
    if(m_useCustom) {
        return m_customMesh->bound();
    } else if(m_mesh) {
        return m_mesh->bound();
    }
    return Renderable::localBound();
}
/*!
    Returns a sprite sheet.
*/
Sprite *SpriteRender::sprite() const {
    return m_sheet;
}
/*!
    Replaces current sprite \a sheet with a new one.
*/
void SpriteRender::setSprite(Sprite *sheet) {
    if(m_sheet != sheet) {
        if(m_sheet) {
            m_sheet->unsubscribe(this);
        }

        if(m_texture) {
            m_texture->decRef();
        }

        m_sheet = sheet;
        if(m_sheet) {
            m_sheet->subscribe(&SpriteRender::spriteUpdated, this);

            m_dirtyMesh = m_dirtyMaterial = true;
        }
    }
}
/*!
    Returns current assigned texture.
*/
Texture *SpriteRender::texture() const {
    if(m_sheet && m_sheet->state() == Resource::Ready) {
        return m_sheet->page();
    }

    return m_texture;
}
/*!
    Replaces current \a texture with a new one.
*/
void SpriteRender::setTexture(Texture *texture) {
    if(m_sheet) {
        m_sheet->unsubscribe(this);
        m_sheet = nullptr;
    }

    m_texture = texture;
    if(!m_materials.empty()) {
        if(m_texture) {
            m_texture->incRef();
        }

        m_dirtyMesh = m_dirtyMaterial = true;
    }
}
/*!
    Returns the color of the sprite to be drawn.
*/
Vector4 SpriteRender::color() const {
    return m_color;
}
/*!
    Changes the \a color of the sprite to be drawn.
*/
void SpriteRender::setColor(const Vector4 &color) {
    m_color = color;

    m_dirtyMaterial = true;
}
/*!
    Returns the current item name of sprite from the sprite sheet.
*/
TString SpriteRender::item() const {
    return m_item;
}
/*!
    Sets the current sub \a item name of sprite from the sprite sheet.
*/
void SpriteRender::setItem(const TString &item) {
    if(m_item != item) {
        m_item = item;
        m_dirtyMesh = true;
    }
}
/*!
    Returns size of sprite.
*/
Vector2 SpriteRender::size() const {
    return m_size;
}
/*!
    Sets a new \a size of sprite.
*/
void SpriteRender::setSize(const Vector2 &size) {
    m_size = size;
}
/*!
    Returns a draw mode for the sprite.
    Please check SpriteRender::DrawMode for more details.
*/
int SpriteRender::drawMode() const {
    return m_drawMode;
}
/*!
    Sets a draw \a mode for the sprite.
    Please check SpriteRender::DrawMode for more details.
*/
void SpriteRender::setDrawMode(int mode) {
    if(m_drawMode != mode) {
        m_drawMode = mode;
        m_dirtyMesh = true;
    }
}
/*!
    Returns the redering layer for the sprite.
*/
int SpriteRender::layer() const {
    return m_priority;
}
/*!
    Sets the redering \a layer for the sprite.
*/
void SpriteRender::setLayer(int layer) {
    m_priority = layer;

    m_dirtyMaterial = true;
}
/*!
    \internal
*/
void SpriteRender::setMaterial(Material *material) {
    Renderable::setMaterial(material);

    m_dirtyMaterial = true;
}
/*!
    \internal
*/
void SpriteRender::setMaterialsList(const std::list<Material *> &materials) {
    Renderable::setMaterialsList(materials);

    m_dirtyMaterial = true;
}
/*!
    \internal
*/
void SpriteRender::composeComponent() {
    setMaterial(Engine::loadResource<Material>(gDefaultSprite));
}
/*!
    \internal
*/
void SpriteRender::spriteUpdated(int state, void *ptr) {
    SpriteRender *p = static_cast<SpriteRender *>(ptr);

    switch(state) {
    case Resource::Ready: {
        p->m_dirtyMesh = p->m_dirtyMaterial = true;
    } break;
    case Resource::ToBeDeleted: {
        p->m_sheet = nullptr;
        p->m_dirtyMesh = p->m_dirtyMaterial = true;
    } break;
    default: break;
    }
}
