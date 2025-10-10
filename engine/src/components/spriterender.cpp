#include "components/spriterender.h"
#include "components/actor.h"
#include "components/transform.h"

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
        m_priority(0),
        m_useCustom(false),
        m_dirty(true) {

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
    if(m_dirty) {
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

        m_dirty = false;
    }

    return (m_useCustom) ? m_customMesh : m_mesh;
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

            m_dirty = true;
            if(!m_materials.empty() && m_sheet->state() == Resource::Ready) {
                m_materials[0]->setTexture(gTexture, m_sheet->page());
            }
        }
    }
}
/*!
    Returns current assigned texture.
*/
Texture *SpriteRender::texture() const {
    if(m_sheet) {
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

        m_dirty = true;
        if(!m_materials.empty()) {
            m_materials[0]->setTexture(gTexture, m_texture);
        }
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

    for(auto it : m_materials) {
        it->setVector4(gColor, &m_color);
    }
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
        m_dirty = true;
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
        m_dirty = true;
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

    for(auto it : m_materials) {
        it->setPriority(m_priority);
    }
}
/*!
    \internal
*/
void SpriteRender::setMaterial(Material *material) {
    Renderable::setMaterial(material);

    for(auto it : m_materials) {
        it->setTexture(gTexture, texture());
        it->setVector4(gColor, &m_color);
        it->setTransform(transform());
    }
}
/*!
    \internal
*/
void SpriteRender::setMaterialsList(const std::list<Material *> &materials) {
    Renderable::setMaterialsList(materials);

    for(auto it : m_materials) {
        it->setTexture(gTexture, texture());
        it->setVector4(gColor, &m_color);
        it->setTransform(transform());
    }
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
        if(!p->m_materials.empty()) {
            p->m_materials[0]->setTexture(gTexture, p->m_sheet->page());
        }
        p->m_dirty = true;
    } break;
    case Resource::ToBeDeleted: {
        p->m_sheet = nullptr;
        if(!p->m_materials.empty()) {
            p->m_materials[0]->setTexture(gTexture, nullptr);
        }
        p->m_dirty = true;
    } break;
    default: break;
    }
}
