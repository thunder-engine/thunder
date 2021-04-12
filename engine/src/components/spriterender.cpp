#include "components/spriterender.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/sprite.h"

#include "commandbuffer.h"

#include <math.h>

#define MATERIAL    "Material"
#define BASEMAP     "BaseMap"

#define OVERRIDE "uni.texture0"
#define COLOR "uni.color0"

static hash<string> hash_str;

class SpriteRenderPrivate : public Resource::IObserver {
public:
    SpriteRenderPrivate() :
            m_pSprite(nullptr),
            m_pTexture(nullptr),
            m_pMaterial(nullptr),
            m_pMesh(Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001")),
            m_pCustomMesh(nullptr),
            m_Color(1.0f),
            m_Size(1.0f),
            m_Hash(0),
            m_DrawMode(0),
            m_Layer(0) {

    }

    ~SpriteRenderPrivate() {
        if(m_pSprite) {
            m_pSprite->unsubscribe(this);
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override {
        if(resource == m_pSprite){
            if(state == Resource::Ready) {
                m_pMaterial->setTexture(OVERRIDE, m_pSprite->texture());
                composeMesh();
            } else if(state == Resource::ToBeDeleted) {
                m_pSprite = nullptr;
                m_pMaterial->setTexture(OVERRIDE, nullptr);
                composeMesh();
            }
        }
    }

    void composeMesh(bool resetSize = false) {
        if(m_pSprite) {
            if(m_pCustomMesh == nullptr) {
                m_pCustomMesh = Engine::objectCreate<Mesh>("");
            }

            bool result = m_pSprite->composeMesh(m_Hash, m_pCustomMesh, m_Size, (m_DrawMode == SpriteRender::Tiled), resetSize);
            if(result) {
                return;
            }
        }
        delete m_pCustomMesh;
        m_pCustomMesh = nullptr;
    }

    Sprite *m_pSprite;

    Texture *m_pTexture;

    MaterialInstance *m_pMaterial;

    Mesh *m_pMesh;
    Mesh *m_pCustomMesh;

    Vector4 m_Color;

    Vector2 m_Size;

    string m_Item;
    int m_Hash;

    int m_DrawMode;

    int m_Layer;
};
/*!
    \class SpriteRender
    \brief Draws a sprite for the 2D graphics.
    \inmodule Engine

    The SpriteRender component allows you to display images as sprites to use in both 2D and 3D scenes.
*/

/*!
    \enum SpriteRender::DrawMode

    \value Sliced \c The SpriteRender will respect 9 patch slicing rules for the size property.
    \value Tiled \c The SpriteRender will duplicate sprites to fill the size property rectangle.
*/

SpriteRender::SpriteRender() :
        p_ptr(new SpriteRenderPrivate) {

}

SpriteRender::~SpriteRender() {
    delete p_ptr;
}
/*!
    \internal
*/
void SpriteRender::draw(ICommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(p_ptr->m_pMesh && layer & a->layers()) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(ICommandBuffer::idToColor(a->uuid()));
        }

        buffer.drawMesh(a->transform()->worldTransform(),
                        (p_ptr->m_pCustomMesh) ? p_ptr->m_pCustomMesh : p_ptr->m_pMesh,
                        layer, p_ptr->m_pMaterial);
        buffer.setColor(Vector4(1.0f));
    }
}
/*!
    \internal
*/
AABBox SpriteRender::bound() const {
    AABBox result = Renderable::bound();
    if(p_ptr->m_pCustomMesh) {
        result = p_ptr->m_pCustomMesh->bound();
    } else if(p_ptr->m_pMesh) {
        result = p_ptr->m_pMesh->bound();
    }
    result = result * actor()->transform()->worldTransform();

    return result;
}
/*!
    Returns an instantiated Material assigned to SpriteRender.
*/
Material *SpriteRender::material() const {
    if(p_ptr->m_pMaterial) {
        return p_ptr->m_pMaterial->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void SpriteRender::setMaterial(Material *material) {
    if(!p_ptr->m_pMaterial || p_ptr->m_pMaterial->material() != material) {
        if(p_ptr->m_pMaterial) {
            delete p_ptr->m_pMaterial;
            p_ptr->m_pMaterial = nullptr;
        }

        if(material) {
            p_ptr->m_pMaterial = material->createInstance();
            p_ptr->m_pMaterial->setTexture(OVERRIDE, texture());
            p_ptr->m_pMaterial->setVector4(COLOR, &p_ptr->m_Color);
        }
    }
}
/*!
    Returns a sprite.
*/
Sprite *SpriteRender::sprite() const {
    return p_ptr->m_pSprite;
}
/*!
    Replaces current \a sprite with a new one.
*/
void SpriteRender::setSprite(Sprite *sprite) {
    if(p_ptr->m_pSprite) {
        p_ptr->m_pSprite->unsubscribe(p_ptr);
    }
    p_ptr->m_pSprite = sprite;
    if(p_ptr->m_pSprite) {
        p_ptr->m_pSprite->subscribe(p_ptr);
        p_ptr->composeMesh();
        if(p_ptr->m_pMaterial) {
            p_ptr->m_pMaterial->setTexture(OVERRIDE, texture());
        }
    }
}
/*!
    Returns current assigned texture.
*/
Texture *SpriteRender::texture() const {
    if(p_ptr->m_pSprite) {
        return p_ptr->m_pSprite->texture();
    }
    return p_ptr->m_pTexture;
}
/*!
    Replaces current \a texture with a new one.
*/
void SpriteRender::setTexture(Texture *texture) {
    p_ptr->m_pTexture = texture;
    if(p_ptr->m_pMaterial) {
        p_ptr->composeMesh();
        p_ptr->m_pMaterial->setTexture(OVERRIDE, SpriteRender::texture());
    }
}
/*!
    Returns the color of the sprite to be drawn.
*/
Vector4 &SpriteRender::color() const {
    return p_ptr->m_Color;
}
/*!
    Changes the \a color of the sprite to be drawn.
*/
void SpriteRender::setColor(const Vector4 &color) {
    p_ptr->m_Color = color;
    if(p_ptr->m_pMaterial) {
        p_ptr->m_pMaterial->setVector4(COLOR, &p_ptr->m_Color);
    }
}
/*!
    Returns the current item name of sprite from the sprite sheet.
*/
string SpriteRender::item() const {
    return p_ptr->m_Item;
}
/*!
    Sets the current sub \a item name of sprite from the sprite sheet.
*/
void SpriteRender::setItem(const string &item) {
    p_ptr->m_Item = item;
    p_ptr->m_Hash = hash_str(p_ptr->m_Item);
    p_ptr->composeMesh(true);
}
/*!
    Returns size of sprite.
*/
Vector2 &SpriteRender::size() const {
    return p_ptr->m_Size;
}
/*!
    Sets a new \a size of sprite.
*/
void SpriteRender::setSize(const Vector2 &size) {
    p_ptr->m_Size = size;
    p_ptr->composeMesh();
}
/*!
    Returns a draw mode for the sprite.
    Please check SpriteRender::DrawMode for more details.
*/
int SpriteRender::drawMode() const {
    return p_ptr->m_DrawMode;
}
/*!
    Sets a draw \a mode for the sprite.
    Please check SpriteRender::DrawMode for more details.
*/
void SpriteRender::setDrawMode(int mode) {
    p_ptr->m_DrawMode = mode;
    p_ptr->composeMesh();
}
/*!
    Returns the order layer for the sprite.
*/
int SpriteRender::layer() const {
    return p_ptr->m_Layer;
}
/*!
    Sets the order \a layer for the sprite.
*/
void SpriteRender::setLayer(int layer) {
    p_ptr->m_Layer = layer;
}
/*!
    \internal
*/
void SpriteRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(MATERIAL);
        if(it != data.end()) {
            setMaterial(Engine::loadResource<Material>((*it).second.toString()));
        }
    }
    {
        auto it = data.find(BASEMAP);
        if(it != data.end()) {
            setSprite(Engine::loadResource<Sprite>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap SpriteRender::saveUserData() const {
    VariantMap result = Component::saveUserData();
    {
        Material *m = material();
        string ref = Engine::reference(m);
        if(!ref.empty()) {
            result[MATERIAL] = ref;
        }
    }
    {
        Sprite *t = sprite();
        string ref = Engine::reference(t);
        if(!ref.empty()) {
            result[BASEMAP] = ref;
        }
    }
    return result;
}
