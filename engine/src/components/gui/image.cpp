#include "components/gui/image.h"

#include "components/gui/recttransform.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/sprite.h"

#include <components/actor.h>
#include <components/spriterender.h>
#include <commandbuffer.h>

namespace {
    const char *gMaterial = "Material";
    const char *gBasemap = "BaseMap";

    const char *gOverride = "texture0";
    const char *gColor = "uni.color0";
    const char *gDefaultSprite = ".embedded/DefaultSprite.mtl";
};

static hash<string> hash_str;

Image::Image() :
        m_color(1.0f),
        m_mesh(Engine::objectCreate<Mesh>("")),
        m_material(nullptr),
        m_customMaterial(nullptr),
        m_sprite(nullptr),
        m_hash(0),
        m_drawMode(Sliced) {

    Material *m = Engine::loadResource<Material>(gDefaultSprite);
    if(m) {
        m_material = m->createInstance();
        m_material->setVector4(gColor, &m_color);
    }
}

Image::~Image() {
    delete m_material;
}

/*!
    \internal
*/
void Image::draw(CommandBuffer &buffer, uint32_t layer) {
    if(m_mesh) {
        Vector3Vector &verts = m_mesh->vertices();
        if(!verts.empty()) {
            Matrix4 mat(rectTransform()->worldTransform());
            mat[12] -= verts[0].x;
            mat[13] -= verts[0].y;

            buffer.setObjectId(actor()->uuid());
            buffer.setMaterialId((m_customMaterial) ? m_customMaterial->material()->uuid() : m_material->material()->uuid());

            buffer.drawMesh(mat, m_mesh, 0, layer, (m_customMaterial) ? m_customMaterial : m_material);
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
            if(m_sprite) {
                m_customMaterial->setTexture(gOverride, m_sprite->texture());
            }
        }
    }
}
/*!
    Returns a sprite.
*/
Sprite *Image::sprite() const {
    return m_sprite;
}
/*!
    Replaces current \a sprite with a new one.
*/
void Image::setSprite(Sprite *sprite) {
    if(m_sprite) {
        m_sprite->unsubscribe(this);
    }
    m_sprite = sprite;
    if(m_sprite) {
        m_sprite->subscribe(&Image::spriteUpdated, this);
        composeMesh();
        setTexture(m_sprite->texture());
    }
}
/*!
    Replaces current \a image with a new one.
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
void Image::setDrawMode(int type) {
    m_drawMode = type;
    composeMesh();
}

/*!
    \internal
*/
void Image::boundChanged(const Vector2 &size) {
    m_meshSize = size;
    composeMesh();
}
/*!
    \internal
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
*/
VariantMap Image::saveUserData() const {
    VariantMap result = Component::saveUserData();
    {
        Material *m = material();
        string ref = Engine::reference(m);
        if(!ref.empty()) {
            result[gMaterial] = ref;
        }
    }
    {
        Sprite *t = sprite();
        string ref = Engine::reference(t);
        if(!ref.empty()) {
            result[gBasemap] = ref;
        }
    }
    return result;
}
/*!
    \internal
*/
void Image::composeMesh() {
    if(m_mesh) {
        SpriteRender::composeMesh(m_sprite, m_hash, m_mesh, m_meshSize, m_drawMode, false, 100.0f);
    }
}
/*!
    \internal
*/
void Image::spriteUpdated(int state, void *ptr) {
    Image *p = static_cast<Image *>(ptr);

    if(state == ResourceState::Ready) {
        if(p->m_customMaterial) {
            p->m_customMaterial->setTexture(gOverride, p->m_sprite->texture());
        }
        p->composeMesh();
    } else if(state == ResourceState::ToBeDeleted) {
        p->m_sprite = nullptr;
        p->m_material->setTexture(gOverride, nullptr);

        if(p->m_customMaterial) {
            p->m_customMaterial->setTexture(gOverride, nullptr);
        }
        p->composeMesh();
    }
}
