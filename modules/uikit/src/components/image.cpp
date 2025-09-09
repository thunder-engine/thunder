#include "components/image.h"

#include "components/recttransform.h"

#include <resources/mesh.h>
#include <resources/material.h>
#include <resources/sprite.h>

#include <commandbuffer.h>
#include <pipelinecontext.h>

namespace {
    const char *gOverride("mainTexture");
    const char *gColor("mainColor");
    const char *gDefaultSprite(".embedded/DefaultSprite.shader");
};

/*!
    \class Image
    \brief The Image class represents an image or sprite that can be drawn on the screen.
    \inmodule Gui

    The Image class represents an image or sprite that can be rendered and displayed on the screen within a graphical user interface (GUI).
    It is used to incorporate visual elements into the interface, such as icons, backgrounds, or illustrations, by loading and rendering image files.
*/

Image::Image() :
        m_color(1.0f),
        m_mesh(Engine::objectCreate<Mesh>()),
        m_sheet(nullptr),
        m_material(nullptr),
        m_hash(0),
        m_drawMode(Simple),
        m_dirty(true) {

    Material *material = Engine::loadResource<Material>(gDefaultSprite);
    if(material) {
        m_material = material->createInstance();
        m_material->setVector4(gColor, &m_color);
    }
}

Image::~Image() {
    if(m_sheet) {
        m_sheet->unsubscribe(this);
    }

    delete m_material;
}
/*!
    \internal
*/
void Image::draw(CommandBuffer &buffer) {
    if(m_dirty) {
        if(m_sheet) {
            Mesh *mesh = m_sheet->composeMesh(m_mesh, m_hash, static_cast<Sprite::Mode>(m_drawMode), m_size);
            if(mesh != m_mesh) {
                if(mesh) {
                    m_mesh->setVertices(mesh->vertices());
                    m_mesh->setIndices(mesh->indices());
                    m_mesh->setUv0(mesh->uv0());
                    m_mesh->setColors(mesh->colors());

                    if(m_drawMode == Sprite::Simple) {
                        Vector2 extent(mesh->bound().extent);
                        Vector2 scl(m_size.x / (extent.x * 2.0f), m_size.y / (extent.y * 2.0f));

                        for(auto &it : m_mesh->vertices()) {
                            it.x = it.x * scl.x;
                            it.y = it.y * scl.y;
                        }
                    }
                } else {
                    makeDefaultMesh();
                }
            }
        } else {
            makeDefaultMesh();
        }

        m_dirty = false;
    }

    RectTransform *rect = rectTransform();
    Matrix4 mat(rect->worldTransform());

    const Vector3Vector &verts(m_mesh->vertices());
    Vector2 scl(rect->worldScale());

    mat[12] -= verts[0].x * scl.x;
    mat[13] -= verts[0].y * scl.y;

    m_material->setTransform(mat);

    buffer.drawMesh(m_mesh, 0, Material::Translucent, *m_material);

    Widget::draw(buffer);
}
/*!
    Returns an instantiated Material assigned to Image.
*/
Material *Image::material() const {
    if(m_material) {
        return m_material->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void Image::setMaterial(Material *material) {
    if(!m_material || m_material->material() != material) {
        if(m_material) {
            delete m_material;
            m_material = nullptr;
        }

        if(material) {
            m_material = material->createInstance();
            m_material->setVector4(gColor, &m_color);
            if(m_sheet) {
                m_material->setTexture(gOverride, m_sheet->page());
            }
        }
    }
}
/*!
    Returns the sprite assigned to the Image.
*/
Sprite *Image::sprite() const {
    return m_sheet;
}
/*!
    Replaces the current sprite \a sheet with a new one.
*/
void Image::setSprite(Sprite *sheet) {
    if(m_sheet != sheet) {
        if(m_sheet) {
            m_sheet->unsubscribe(this);
        }

        m_sheet = sheet;
        if(m_sheet) {
            m_sheet->subscribe(&Image::spriteUpdated, this);

            setTexture(m_sheet->page());
        }
    }
}
/*!
    Replaces the current \a image with a new one.
*/
void Image::setTexture(Texture *image) {
    if(image) {
        image->incRef();
    }

    if(m_material) {
        m_material->setTexture(gOverride, image);
    }
}
/*!
    Returns the color of the image to be drawn.
*/
Vector4 Image::color() const {
    return m_color;
}
/*!
    Changes the \a color of the image to be drawn.
*/
void Image::setColor(const Vector4 &color) {
    m_color = color;
    if(m_material) {
        m_material->setVector4(gColor, &m_color);
    }
}
/*!
    Returns the current item name of sprite from the sprite sheet.
*/
TString Image::item() const {
    return m_item;
}
/*!
    Sets the current sub \a item name of sprite from the sprite sheet.
*/
void Image::setItem(const TString &item) {
    if(m_item != item) {
        m_item = item;
        m_hash = Mathf::hashString(m_item);
        m_dirty = true;
    }
}
/*!
    Returns a draw mode for the image.
    Please check Image::DrawMode for more details.
*/
int Image::drawMode() const {
    return m_drawMode;
}
/*!
    Sets a draw \a mode for the image.
    Please check Image::DrawMode for more details.
*/
void Image::setDrawMode(int mode) {
    if(m_drawMode != mode) {
        m_drawMode = mode;
        m_dirty = true;
    }
}
/*!
    \internal
    Callback method called when the \a size of the image change. Recomposes the mesh based on new bounds.
*/
void Image::boundChanged(const Vector2 &size) {
    Widget::boundChanged(size);

    if(m_size != size) {
        m_size = size;
        m_dirty = true;
    }
}
/*!
    \internal
*/
void Image::makeDefaultMesh() {
    m_mesh->setVertices({
        {-m_size.x * 0.5f,-m_size.y * 0.5f, 0.0f},
        {-m_size.x * 0.5f, m_size.y * 0.5f, 0.0f},
        { m_size.x * 0.5f, m_size.y * 0.5f, 0.0f},
        { m_size.x * 0.5f,-m_size.y * 0.5f, 0.0f},
    });
    m_mesh->setUv0({
        {0.0f, 0.0f},
        {0.0f, 1.0f},
        {1.0f, 1.0f},
        {1.0f, 0.0f},
    });
    m_mesh->setColors({
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
    });
    m_mesh->setIndices({0, 1, 2, 0, 3, 2});
    m_mesh->recalcBounds();
}
/*!
    \internal
    Callback method called when the sprite is updated. Handles updating the mesh and material based on sprite changes.
*/
void Image::spriteUpdated(int state, void *ptr) {
    Image *p = static_cast<Image *>(ptr);

    switch(state) {
        case Resource::Ready: {
            if(p->m_material) {
                p->m_material->setTexture(gOverride, p->m_sheet->page());
            }

            p->m_dirty = true;
        } break;
        case Resource::ToBeDeleted: {
            p->m_sheet = nullptr;
            if(p->m_material) {
                p->m_material->setTexture(gOverride, nullptr);
            }

            p->m_dirty = true;
        } break;
        default: break;
    }
}
