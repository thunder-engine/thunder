#include "components/spriterender.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/sprite.h"

#include "commandbuffer.h"
#include "pipelinecontext.h"

#include <math.h>

namespace  {
    const char *gBaseMap = "BaseMap";

    const char *gColor = "mainColor";
    const char *gTexture = "mainTexture";
    const char *gDefaultSprite = ".embedded/DefaultSprite.shader";
}

static std::hash<std::string> hash_str;

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
        m_customMesh(nullptr),
        m_hash(0),
        m_drawMode(Simple),
        m_layer(0) {

}

SpriteRender::~SpriteRender() {
    if(m_sheet) {
        m_sheet->unsubscribe(this);
    }

    if(m_customMesh) {
        m_customMesh->decRef();
    }
}
/*!
    \internal
*/
Mesh *SpriteRender::meshToDraw() const {
    return (m_customMesh) ? m_customMesh : m_mesh;
}
/*!
    \internal
*/
AABBox SpriteRender::localBound() const {
    if(m_customMesh) {
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

        m_sheet = sheet;
        if(m_sheet) {
            m_sheet->subscribe(&SpriteRender::spriteUpdated, this);

            composeMesh();
            if(!m_materials.empty()) {
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

        composeMesh();
        m_materials[0]->setTexture(gTexture, m_texture);
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
void SpriteRender::setColor(const Vector4 color) {
    m_color = color;

    for(auto it : m_materials) {
        it->setVector4(gColor, &m_color);
    }
}
/*!
    Returns the current item name of sprite from the sprite sheet.
*/
std::string SpriteRender::item() const {
    return m_item;
}
/*!
    Sets the current sub \a item name of sprite from the sprite sheet.
*/
void SpriteRender::setItem(const std::string item) {
    m_item = item;
    m_hash = hash_str(m_item);
    composeMesh(true);
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
void SpriteRender::setSize(const Vector2 size) {
    m_size = size;
    composeMesh();
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
    m_drawMode = mode;
    composeMesh();
}
/*!
    Returns the order layer for the sprite.
*/
int SpriteRender::layer() const {
    return m_layer;
}
/*!
    Sets the order \a layer for the sprite.
*/
void SpriteRender::setLayer(int layer) {
    m_layer = layer;
}
/*!
    \internal
*/
void SpriteRender::loadUserData(const VariantMap &data) {
    Renderable::loadUserData(data);

    auto it = data.find(gBaseMap);
    if(it != data.end()) {
        setSprite(Engine::loadResource<Sprite>((*it).second.toString()));
    }
}
/*!
    \internal
*/
VariantMap SpriteRender::saveUserData() const {
    VariantMap result(Renderable::saveUserData());

    Sprite *t = sprite();
    std::string ref = Engine::reference(t);
    if(!ref.empty()) {
        result[gBaseMap] = ref;
    }

    return result;
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

Mesh *SpriteRender::composeMesh(Sprite *sprite, int key, Vector2 &size, int mode, bool resetSize, float scale) {
    Mesh *result = nullptr;

    if(sprite) {
        result = sprite->shape(key);
        if(result) {
            if(mode == Sliced || mode == Tiled) {
                Mesh *spriteMesh = Engine::objectCreate<Mesh>("");

                spriteMesh->setVertices(result->vertices());
                spriteMesh->setIndices(result->indices());
                spriteMesh->setColors(result->colors());
                spriteMesh->setUv0(result->uv0());

                Vector3Vector &vertices = spriteMesh->vertices();
                Vector3 delta(vertices[15] * scale - vertices[0] * scale);
                if(resetSize) {
                    size = Vector2(delta.x, delta.y);
                }

                if(mode == Sliced && !composeSliced(spriteMesh, size, delta, scale)) {
                    return spriteMesh;
                } else if(mode == Tiled && !composeTiled(spriteMesh, size, delta, scale)) {
                    return spriteMesh;
                }

                if(spriteMesh->colors().empty()) {
                    spriteMesh->setColors(Vector4Vector(vertices.size(), Vector4(1.0f)));
                }

                spriteMesh->recalcBounds();

                return spriteMesh;
            }
        }
    } else {
        result = Engine::objectCreate<Mesh>();
        result->setVertices({
            {  0.0f,   0.0f, 0.0f},
            {  0.0f, size.y, 0.0f},
            {size.x, size.y, 0.0f},
            {size.x,   0.0f, 0.0f},
        });
        result->setUv0({
            {0.0f, 0.0f},
            {0.0f, 1.0f},
            {1.0f, 1.0f},
            {1.0f, 0.0f},
        });
        result->setColors({
            {1.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
        });
        result->setIndices({0, 1, 2, 0, 3, 2});
        result->recalcBounds();
    }

    return result;
}
/*!
    \internal
*/
bool SpriteRender::composeSliced(Mesh *mesh, Vector2 &size, Vector3 &delta, float scale) {
    Vector3Vector &verts = mesh->vertices();

    if(scale != 1.0f) {
        for(int i = 0; i < 16; i++) {
            verts[i] *= scale;
        }
    }

    Vector2 scl(size.x / delta.x, size.y / delta.y);
    {
        float wl = (verts[1].x - verts[0].x);
        float wr = (verts[3].x - verts[2].x);
        float borders = wl + wr;

        float vl = verts[0].x * scl.x;
        float vr = verts[3].x * scl.x;

        verts[ 0].x = vl; verts[ 3].x = vr;
        verts[ 4].x = vl; verts[ 7].x = vr;
        verts[ 8].x = vl; verts[11].x = vr;
        verts[12].x = vl; verts[15].x = vr;

        float dl = verts[0].x + MIN(size.x * (wl / borders), wl);
        float dr = verts[3].x - MIN(size.x * (wr / borders), wr);

        verts[ 1].x = dl; verts[ 2].x = dr;
        verts[ 5].x = dl; verts[ 6].x = dr;
        verts[ 9].x = dl; verts[10].x = dr;
        verts[13].x = dl; verts[14].x = dr;
    }
    {
        float hb = (verts[ 4].y - verts[0].y);
        float ht = (verts[12].y - verts[8].y);
        float borders = hb + ht;

        float vb = verts[ 0].y * scl.y;
        float vt = verts[12].y * scl.y;

        verts[ 0].y = vb; verts[12].y = vt;
        verts[ 1].y = vb; verts[13].y = vt;
        verts[ 2].y = vb; verts[14].y = vt;
        verts[ 3].y = vb; verts[15].y = vt;

        float db = verts[ 0].y + MIN(size.y * (hb / borders), hb);
        float dt = verts[12].y - MIN(size.y * (ht / borders), ht);

        verts[ 4].y = db; verts[ 8].y = dt;
        verts[ 5].y = db; verts[ 9].y = dt;
        verts[ 6].y = db; verts[10].y = dt;
        verts[ 7].y = db; verts[11].y = dt;
    }
    return true;
}
/*!
    \internal
*/
bool SpriteRender::composeTiled(Mesh *mesh, Vector2 &size, Vector3 &delta, float scale) {
    Vector3Vector &verts = mesh->vertices();
    Vector2Vector &uvs = mesh->uv0();
    Vector4Vector &colors = mesh->colors();
    IndexVector &indices = mesh->indices();

    Vector2 ubl(uvs[0]);
    Vector2 utr(uvs[15]);

    int width = ceilf(size.x / delta.x);
    int height = ceilf(size.y / delta.y);

    if(width == 0 || height == 0) {
        return false;
    }

    verts.resize(width * height * 4);
    uvs.resize(width * height * 4);
    colors.resize(width * height * 4, Vector4(1.0f));
    indices.resize(width * height * 6);

    Vector3 bl(Vector3(size, 0.0f) * -0.5f);

    int i = 0;
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;

            Vector2 f(1.0f);
            if(x == width - 1) {
                f.x = MIN((size.x * 0.5f - bl.x) / delta.x, 1.0f);
            }
            if(y == height - 1) {
                f.y = MIN((size.y * 0.5f - bl.y) / delta.y, 1.0f);
            }

            verts[index] = bl;
            verts[index + 1] = bl + Vector3(delta.x * f.x, 0.0f, 0.0f);
            verts[index + 2] = bl + Vector3(delta.x * f.x, delta.y * f.y, 0.0f);
            verts[index + 3] = bl + Vector3(0.0f, delta.y * f.y, 0.0f);

            uvs[index] = ubl;
            uvs[index + 1] = ubl + Vector2((utr.x - ubl.x) * f.x, 0.0f);
            uvs[index + 2] = ubl + Vector2((utr.x - ubl.x) * f.x, (utr.y - ubl.y) * f.y);
            uvs[index + 3] = ubl + Vector2(0.0f, (utr.y - ubl.y) * f.y);

            indices[i] = index;
            indices[i + 1] = index + 1;
            indices[i + 2] = index + 2;
            indices[i + 3] = index;
            indices[i + 4] = index + 2;
            indices[i + 5] = index + 3;

            bl.x += delta.x;

            i += 6;
        }
        bl.x = size.x * -0.5f;
        bl.y += delta.y;
    }
    return true;
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
int SpriteRender::priority() const {
    return m_layer;
}
/*!
    \internal
*/
void SpriteRender::composeMesh(bool resetSize) {
    if(m_sheet) {
        Mesh *mesh = SpriteRender::composeMesh(m_sheet, m_hash, m_size, m_drawMode, resetSize);
        if(mesh != m_customMesh) {
            if(m_customMesh) {
                m_customMesh->decRef();
            }
            m_customMesh = mesh;
            if(m_customMesh) {
                m_customMesh->incRef();
            }
        }
    }

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
        p->composeMesh();
    } break;
    case Resource::ToBeDeleted: {
        p->m_sheet = nullptr;
        if(!p->m_materials.empty()) {
            p->m_materials[0]->setTexture(gTexture, nullptr);
        }
        p->composeMesh();
    } break;
    default: break;
    }
}
