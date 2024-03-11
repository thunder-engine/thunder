#include "components/image.h"

#include "components/recttransform.h"

#include <resources/mesh.h>
#include <resources/material.h>
#include <resources/sprite.h>

#include <components/actor.h>
#include <components/spriterender.h>
#include <commandbuffer.h>

namespace {
    const char *gMaterial = "Material";
    const char *gBasemap = "BaseMap";

    const char *gOverride = "texture0";
    const char *gColor = "color0";
    const char *gDefaultSprite = ".embedded/DefaultSprite.mtl";
};

static hash<string> hash_str;

/*!
    \class Image
    \brief The Image class represents an image or sprite that can be drawn on the screen.
    \inmodule Gui
*/

Image::Image() :
        m_color(1.0f),
        m_mesh(Engine::objectCreate<Mesh>("")),
        m_material(nullptr),
        m_customMaterial(nullptr),
        m_sheet(nullptr),
        m_hash(0),
        m_drawMode(Sliced) {

    Material *material = Engine::loadResource<Material>(gDefaultSprite);
    if(material) {
        m_material = material->createInstance();
        m_material->setVector4(gColor, &m_color);
    }
}

Image::~Image() {
    delete m_material;
}

/*!
    \internal
*/
void Image::draw(CommandBuffer &buffer) {
    if(m_mesh) {
        Vector3Vector &verts = m_mesh->vertices();
        if(!verts.empty()) {
            Matrix4 mat(rectTransform()->worldTransform());
            mat[12] -= verts[0].x;
            mat[13] -= verts[0].y;

            buffer.setObjectId(actor()->uuid());
            buffer.setMaterialId((m_customMaterial) ? m_customMaterial->material()->uuid() : m_material->material()->uuid());

            buffer.drawMesh(mat, m_mesh, 0, CommandBuffer::UI, (m_customMaterial) ? m_customMaterial : m_material);
        }
    }
}

/*!
    Returns an instantiated Material assigned to Image.
*/
Material *Image::material() const {
    if(m_customMaterial) {
        return m_customMaterial->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void Image::setMaterial(Material *material) {
    if(!m_customMaterial || m_customMaterial->material() != material) {
        if(m_customMaterial) {
            delete m_customMaterial;
            m_customMaterial = nullptr;
        }

        if(material) {
            m_customMaterial = material->createInstance();
            m_customMaterial->setVector4(gColor, &m_color);
            if(m_sheet) {
                m_customMaterial->setTexture(gOverride, m_sheet->page());
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
    if(m_sheet) {
        m_sheet->unsubscribe(this);
    }
    m_sheet = sheet;
    if(m_sheet) {
        m_sheet->subscribe(&Image::spriteUpdated, this);
        composeMesh();
        setTexture(m_sheet->page());
    }
}
/*!
    Replaces the current \a image with a new one.
*/
void Image::setTexture(Texture *image) {
    if(m_material) {
        m_material->setTexture(gOverride, image);
    }
    if(m_customMaterial) {
        m_customMaterial->setTexture(gOverride, image);
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
void Image::setColor(const Vector4 color) {
    m_color = color;
    if(m_customMaterial) {
        m_customMaterial->setVector4(gColor, &m_color);
    } else if(m_material) {
        m_material->setVector4(gColor, &m_color);
    }
}
/*!
    Returns the current item name of sprite from the sprite sheet.
*/
string Image::item() const {
    return m_item;
}
/*!
    Sets the current sub \a item name of sprite from the sprite sheet.
*/
void Image::setItem(const string item) {
    m_item = item;
    m_hash = hash_str(m_item);
    composeMesh();
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
    m_drawMode = mode;
    composeMesh();
}

/*!
    \internal
    Callback method called when the \a bounds of the image change. Recomposes the mesh based on new bounds.
*/
void Image::boundChanged(const Vector2 &bounds) {
    m_meshSize = bounds;
    composeMesh();
}
/*!
    \internal
    Loads user data for the image.
*/
void Image::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(gMaterial);
        if(it != data.end()) {
            setMaterial(Engine::loadResource<Material>((*it).second.toString()));
        }
    }
    {
        auto it = data.find(gBasemap);
        if(it != data.end()) {
            setSprite(Engine::loadResource<Sprite>((*it).second.toString()));
        }
    }
}
/*!
    \internal
    Saves user data for the image.
*/
VariantMap Image::saveUserData() const {
    VariantMap result = Component::saveUserData();
    {
        string ref = Engine::reference(material());
        if(!ref.empty()) {
            result[gMaterial] = ref;
        }
    }
    {
        string ref = Engine::reference(sprite());
        if(!ref.empty()) {
            result[gBasemap] = ref;
        }
    }
    return result;
}
/*!
    \internal
     Composes the mesh for rendering based on the current sprite, hash, mesh, size, and draw mode.
*/
void Image::composeMesh() {
    if(m_mesh) {
        SpriteRender::composeMesh(m_sheet, m_hash, m_mesh, m_meshSize, m_drawMode, false, 100.0f);
    }
}
/*!
    \internal
    Callback method called when the sprite is updated. Handles updating the mesh and material based on sprite changes.
*/
void Image::spriteUpdated(int state, void *ptr) {
    Image *p = static_cast<Image *>(ptr);

    switch(state) {
    case ResourceState::Ready: {
        if(p->m_customMaterial) {
            p->m_customMaterial->setTexture(gOverride, p->m_sheet->page());
        }
        p->composeMesh();
    } break;
    case ResourceState::ToBeDeleted: {
        p->m_sheet = nullptr;
        p->m_material->setTexture(gOverride, nullptr);

        if(p->m_customMaterial) {
            p->m_customMaterial->setTexture(gOverride, nullptr);
        }
        p->composeMesh();
    } break;
    default: break;
    }
}
