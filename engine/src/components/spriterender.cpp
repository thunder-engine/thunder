#include "components/spriterender.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/sprite.h"

#include "commandbuffer.h"

#define MATERIAL    "Material"
#define BASEMAP     "BaseMap"

#define OVERRIDE "uni.texture0"
#define COLOR "uni.color0"

class SpriteRenderPrivate : public Resource::IObserver {
public:
    SpriteRenderPrivate() :
            m_pSprite(nullptr),
            m_pTexture(nullptr),
            m_pMaterial(nullptr),
            m_pMesh(Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001")),
            m_Color(1.0f),
            m_Index(0) {

    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override {
        if(resource == m_pSprite && state == Resource::Ready) {
            m_pSprite->texture()->setDirty();
        }
    }

    Sprite *m_pSprite;

    Texture *m_pTexture;

    MaterialInstance *m_pMaterial;

    Mesh *m_pMesh;

    Vector4 m_Color;

    int m_Index;
};
/*!
    \class SpriteRender
    \brief Draws a sprite for the 2D graphics.
    \inmodule Engine

    The SpriteRender component allows you to display images as sprites to use in both 2D and 3D scenes.
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

        if(p_ptr->m_pMaterial) {
            p_ptr->m_pMaterial->setTexture(OVERRIDE, texture());
        }
        buffer.drawMesh(a->transform()->worldTransform(), p_ptr->m_pMesh, layer, p_ptr->m_pMaterial);
        buffer.setColor(Vector4(1.0f));
    }
}
/*!
    \internal
*/
AABBox SpriteRender::bound() const {
    if(p_ptr->m_pMesh) {
        return p_ptr->m_pMesh->bound() * actor()->transform()->worldTransform();
    }
    return Renderable::bound();
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
    p_ptr->m_pSprite = sprite;
    if(p_ptr->m_pSprite) {
        p_ptr->m_pSprite->unsubscribe(p_ptr);
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
}
/*!
    Returns the color of the sprite to be drawn.
*/
Vector4 SpriteRender::color() const {
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
    Returns the current index of sprite from the sprite sheet.
*/
int SpriteRender::index() const {
    return p_ptr->m_Index;
}
/*!
    Sets the current \a index of sprite from the sprite sheet.
*/
void SpriteRender::setIndex(int index) {
    p_ptr->m_Index = index;
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
