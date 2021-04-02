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
            Mesh *mesh = m_pSprite->mesh(m_Hash);
            if(mesh) {
                if(m_pCustomMesh == nullptr) {
                    m_pCustomMesh = Engine::objectCreate<Mesh>("");
                }

                Lod *lod = mesh->lod(0);
                m_pCustomMesh->setLod(0, lod);
                lod = m_pCustomMesh->lod(0);
                Vector3Vector &vetrs = lod->vertices();

                Vector3 size(vetrs[15] - vetrs[0]);
                if(resetSize) {
                    m_Size = Vector2(size.x, size.y);
                }

                if(m_DrawMode == SpriteRender::Tiled) {
                    Vector2Vector &uvs = lod->uv0();
                    IndexVector &indices = lod->indices();

                    Vector2 ubl(uvs[0]);
                    Vector2 utr(uvs[15]);

                    int width = ceilf(m_Size.x / size.x);
                    int height = ceilf(m_Size.y / size.y);

                    vetrs.resize(width * height * 4);
                    uvs.resize(width * height * 4);
                    indices.resize(width * height * 6);

                    Vector3 bl(Vector3(m_Size, 0.0f) * -0.5f);

                    int i = 0;
                    for(int y = 0; y < height; y++) {
                        for(int x = 0; x < width; x++) {
                            int index = (y * width + x) * 4;

                            Vector2 f(1.0f);
                            if(x == width - 1) {
                                f.x = MIN((m_Size.x * 0.5f - bl.x) / size.x, 1.0f);
                            }
                            if(y == height - 1) {
                                f.y = MIN((m_Size.y * 0.5f - bl.y) / size.y, 1.0f);
                            }

                            vetrs[index] = bl;
                            vetrs[index + 1] = bl + Vector3(size.x * f.x, 0.0f, 0.0f);
                            vetrs[index + 2] = bl + Vector3(size.x * f.x, size.y * f.y, 0.0f);
                            vetrs[index + 3] = bl + Vector3(0.0f, size.y * f.y, 0.0f);

                            uvs[index] = ubl;
                            uvs[index + 1] = ubl + Vector2((utr.x - ubl.x) * f.x, 0.0f);
                            uvs[index + 2] = ubl + Vector2((utr.x - ubl.x) * f.x, (utr.y - ubl.y) * f.y);
                            uvs[index + 3] = ubl + Vector2(0.0f, (utr.y - ubl.y) * f.y);

                            indices[i]     = index;
                            indices[i + 1] = index + 1;
                            indices[i + 2] = index + 2;
                            indices[i + 3] = index;
                            indices[i + 4] = index + 2;
                            indices[i + 5] = index + 3;

                            bl.x += size.x;

                            i += 6;
                        }
                        bl.y += size.y;
                    }
                } else {
                    Vector2 scale(m_Size.x / size.x, m_Size.y / size.y);
                    {
                        float bl = vetrs[0].x;
                        float br = vetrs[3].x;
                        vetrs[ 0].x *= scale.x; vetrs[ 3].x *= scale.x;
                        vetrs[ 4].x *= scale.x; vetrs[ 7].x *= scale.x;
                        vetrs[ 8].x *= scale.x; vetrs[11].x *= scale.x;
                        vetrs[12].x *= scale.x; vetrs[15].x *= scale.x;
                        float dl = vetrs[0].x - bl;
                        float dr = vetrs[3].x - br;
                        vetrs[ 1].x += dl; vetrs[ 2].x += dr;
                        vetrs[ 5].x += dl; vetrs[ 6].x += dr;
                        vetrs[ 9].x += dl; vetrs[10].x += dr;
                        vetrs[13].x += dl; vetrs[14].x += dr;
                    }
                    {
                        float bl = vetrs[ 0].y;
                        float br = vetrs[12].y;
                        vetrs[ 0].y *= scale.y; vetrs[12].y *= scale.y;
                        vetrs[ 1].y *= scale.y; vetrs[13].y *= scale.y;
                        vetrs[ 2].y *= scale.y; vetrs[14].y *= scale.y;
                        vetrs[ 3].y *= scale.y; vetrs[15].y *= scale.y;
                        float dl = vetrs[ 0].y - bl;
                        float dr = vetrs[12].y - br;
                        vetrs[ 4].y += dl; vetrs[ 8].y += dr;
                        vetrs[ 5].y += dl; vetrs[ 9].y += dr;
                        vetrs[ 6].y += dl; vetrs[10].y += dr;
                        vetrs[ 7].y += dl; vetrs[11].y += dr;
                    }
                }

                m_pCustomMesh->setFlags(mesh->flags());
                m_pCustomMesh->setMode(mesh->mode());
                m_pCustomMesh->recalcBounds();
            }
        } else {
            delete m_pCustomMesh;
            m_pCustomMesh = nullptr;
        }
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
