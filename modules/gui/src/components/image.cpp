#include "components/image.h"

#include "components/recttransform.h"

#include <resources/mesh.h>
#include <resources/material.h>
#include <resources/sprite.h>

#include <components/actor.h>
#include <components/transform.h>
#include <components/spriterender.h>
#include <commandbuffer.h>

#define MATERIAL "Material"
#define BASEMAP "BaseMap"

#define OVERRIDE "texture0"
#define COLOR "uni.color0"

#define DEFAULTSPRITE ".embedded/DefaultSprite.mtl"

static hash<string> hash_str;

class ImagePrivate : public Resource::IObserver {
public:
    explicit ImagePrivate(Image *image) :
        m_Color(1.0f),
        m_pImage(image),
        m_pCustomMesh(Engine::objectCreate<Mesh>("")),
        m_pMaterial(nullptr),
        m_pCustomMaterial(nullptr),
        m_pSprite(nullptr),
        m_Hash(0) {

        Material *m = dynamic_cast<Material *>(Engine::loadResource<Material>(DEFAULTSPRITE));
        if(m) {
            m_pMaterial = m->createInstance();
            m_pMaterial->setVector4(COLOR, &m_Color);
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override {
        if(resource == m_pSprite) {
            if(state == Resource::Ready) {
                if(m_pCustomMaterial) {
                    m_pCustomMaterial->setTexture(OVERRIDE, m_pSprite->texture());
                }
                composeMesh();
            } else if(state == Resource::ToBeDeleted) {
                m_pSprite = nullptr;
                m_pMaterial->setTexture(OVERRIDE, nullptr);
                if(m_pCustomMaterial) {
                    m_pCustomMaterial->setTexture(OVERRIDE, nullptr);
                }
                composeMesh();
            }
        }
    }

    void composeMesh() {
        RectTransform *t = dynamic_cast<RectTransform *>(m_pImage->actor()->transform());
        if(t) {
            if(m_pCustomMesh && m_pSprite) {
                Vector2 size = t->size();
                SpriteRender::composeMesh(m_pSprite, m_Hash, m_pCustomMesh, size, false, false, 100.0f);
            }
        }
    }

    Vector4 m_Color;

    string m_Item;

    Image *m_pImage;

    Mesh *m_pCustomMesh;

    MaterialInstance *m_pMaterial;
    MaterialInstance *m_pCustomMaterial;

    Sprite *m_pSprite;

    int m_Hash;
};

Image::Image() :
    p_ptr(new ImagePrivate(this)) {

}

Image::~Image() {
    delete p_ptr;
    p_ptr = nullptr;
}

/*!
    \internal
*/
void Image::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(p_ptr->m_pCustomMesh) {
        if(layer & CommandBuffer::RAYCAST) {
            buffer.setColor(CommandBuffer::idToColor(a->uuid()));
        }
        buffer.drawMesh(a->transform()->worldTransform(),
                        p_ptr->m_pCustomMesh,
                        0, layer, (p_ptr->m_pCustomMaterial) ? p_ptr->m_pCustomMaterial : p_ptr->m_pMaterial);
        buffer.setColor(Vector4(1.0f));
    }
}

/*!
    Returns an instantiated Material assigned to SpriteRender.
*/
Material *Image::material() const {
    if(p_ptr->m_pCustomMaterial) {
        return p_ptr->m_pCustomMaterial->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void Image::setMaterial(Material *material) {
    if(!p_ptr->m_pCustomMaterial || p_ptr->m_pCustomMaterial->material() != material) {
        if(p_ptr->m_pCustomMaterial) {
            delete p_ptr->m_pCustomMaterial;
            p_ptr->m_pCustomMaterial = nullptr;
        }

        if(material) {
            p_ptr->m_pCustomMaterial = material->createInstance();
            p_ptr->m_pCustomMaterial->setVector4(COLOR, &p_ptr->m_Color);
            if(p_ptr->m_pSprite) {
                p_ptr->m_pCustomMaterial->setTexture(OVERRIDE, p_ptr->m_pSprite->texture());
            }
        }
    }
}
/*!
    Returns a sprite.
*/
Sprite *Image::sprite() const {
    return p_ptr->m_pSprite;
}
/*!
    Replaces current \a sprite with a new one.
*/
void Image::setSprite(Sprite *sprite) {
    if(p_ptr->m_pSprite) {
        p_ptr->m_pSprite->unsubscribe(p_ptr);
    }
    p_ptr->m_pSprite = sprite;
    if(p_ptr->m_pSprite) {
        p_ptr->m_pSprite->subscribe(p_ptr);
        p_ptr->composeMesh();
        if(p_ptr->m_pMaterial) {
            p_ptr->m_pMaterial->setTexture(OVERRIDE, p_ptr->m_pSprite->texture());
        }
        if(p_ptr->m_pCustomMaterial) {
            p_ptr->m_pCustomMaterial->setTexture(OVERRIDE, p_ptr->m_pSprite->texture());
        }
    }
}
/*!
    Returns the color of the image to be drawn.
*/
Vector4 Image::color() const {
    return p_ptr->m_Color;
}
/*!
    Changes the \a color of the image to be drawn.
*/
void Image::setColor(const Vector4 &color) {
    p_ptr->m_Color = color;
    if(p_ptr->m_pCustomMaterial) {
        p_ptr->m_pCustomMaterial->setVector4(COLOR, &p_ptr->m_Color);
    }
}
/*!
    Returns the current item name of sprite from the sprite sheet.
*/
string Image::item() const {
    return p_ptr->m_Item;
}
/*!
    Sets the current sub \a item name of sprite from the sprite sheet.
*/
void Image::setItem(const string &item) {
    p_ptr->m_Item = item;
    p_ptr->m_Hash = hash_str(p_ptr->m_Item);
    p_ptr->composeMesh();
}
/*!
    \internal
*/
void Image::boundChanged() {
    p_ptr->composeMesh();
}
/*!
    \internal
*/
void Image::composeComponent() {
    Widget::composeComponent();

    setSprite(Engine::loadResource<Sprite>(".embedded/ui.png"));
    setItem("Rectangle");
}
/*!
    \internal
*/
void Image::loadUserData(const VariantMap &data) {
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
VariantMap Image::saveUserData() const {
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
