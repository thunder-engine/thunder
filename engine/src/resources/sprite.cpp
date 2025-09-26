#include "sprite.h"

#include "engine.h"
#include "texture.h"
#include "utils/atlas.h"

#include <cstring>

namespace  {
    const char *gPages("Pages");
    const char *gShapes("Shapes");
}

/*!
    \class Sprite
    \brief Represents 2D sprite.
    \inmodule Resources

    Sprites usually used in games to display environment and characters in 2D games.
    This class also supports sprite sheets to contain several images in one container to simplify animation or handle tile maps.
*/

Sprite::Sprite() :
        m_pixelsPerUnit(1.0f) {

}

Sprite::~Sprite() {
    PROFILE_FUNCTION();

    Sprite::clear();
}
/*!
    Adds new sub \a texture as element to current sprite sheet.
    All elements will be packed to a single sprite sheet texture using Sprite::pack() method.
    Returns the id of the new element.

    \sa packSheets()
*/
int Sprite::addElement(Texture *texture) {
    PROFILE_FUNCTION();

    m_sources.push_back(texture);

    Mesh *mesh = Engine::objectCreate<Mesh>();
    mesh->makeDynamic();
    mesh->setVertices({Vector3(0.0f, 0.0f, 0.0f),
                       Vector3(0.0f, texture->height(), 0.0f),
                       Vector3(texture->width(), texture->height(), 0.0f),
                       Vector3(texture->width(), 0.0f, 0.0f) });

    mesh->setIndices({0, 1, 2, 0, 2, 3});

    int index = (m_sources.size() - 1);
    m_shapes[index].mesh = mesh;

    return index;
}
/*!
    Packs all added elements int to a sprite sheets.
    Parameter \a padding can be used to delimit elements.

    \sa addElement()
*/
void Sprite::packSheets(int padding) {
    PROFILE_FUNCTION();

    uint32_t atlasWidth = 1;
    uint32_t atlasHeight = 1;

    std::vector<AtlasNode *> nodes;
    nodes.resize(m_sources.size());

    AtlasNode root;

    if(m_pages.empty()) {
        Texture *texture = Engine::objectCreate<Texture>();
        texture->setFiltering(Texture::Bilinear);
        addPage(texture);
    }

    while(true) {
        root.w = atlasWidth;
        root.h = atlasHeight;

        uint32_t i;
        for(i = 0; i < m_sources.size(); i++) {
            Texture *texture = m_sources[i];

            int32_t width  = (texture->width() + padding * 2);
            int32_t height = (texture->height() + padding * 2);

            AtlasNode *node = root.insert(width, height);
            if(node) {
                node->fill = true;

                nodes[i] = node;
            } else {
                atlasWidth *= 2;
                atlasHeight *= 2;

                if(root.left) {
                    delete root.left;
                    root.left = nullptr;
                }

                if(root.right) {
                    delete root.right;
                    root.right = nullptr;
                }

                root.fill = false;

                break;
            }
        }

        if(i == m_sources.size()) {
            break;
        }
    }

    for(auto it : m_pages) {
        it->resize(atlasWidth, atlasHeight);
        for(uint32_t i = 0; i < nodes.size(); i++) {
            AtlasNode *node = nodes[i];

            int32_t w = node->w - padding * 2;
            int32_t h = node->h - padding * 2;

            uint8_t *src = m_sources[i]->surface(0).front().data();
            uint8_t *dst = it->surface(0).front().data();
            for(int32_t y = 0; y < h; y++) {
                memcpy(&dst[(y + node->y + padding) * atlasWidth + node->x], &src[y * w], w);
            }

            Mesh *mesh = shape(i);
            if(mesh) {
                Vector4 uvFrame;
                uvFrame.x = node->x / static_cast<float>(atlasWidth);
                uvFrame.y = (node->y + padding) / static_cast<float>(atlasHeight);
                uvFrame.z = uvFrame.x + w / static_cast<float>(atlasWidth);
                uvFrame.w = uvFrame.y + h / static_cast<float>(atlasHeight);

                mesh->setUv0({Vector2(uvFrame.x, uvFrame.y),
                              Vector2(uvFrame.z, uvFrame.y),
                              Vector2(uvFrame.z, uvFrame.w),
                              Vector2(uvFrame.x, uvFrame.w)});

                mesh->recalcBounds();
            }
        }

        it->setDirty();
    }
}
/*!
    \internal
*/
void Sprite::loadUserData(const VariantMap &data) {
    clear();
    {
        auto it = data.find(gPages);
        if(it != data.end()) {
            for(auto &page : it->second.toList()) {
                addPage(Engine::loadResource<Texture>(page.toString()));
            }
        }
    }
    {
        auto it = data.find(gShapes);
        if(it != data.end()) {
            for(auto &mesh : it->second.toList()) {
                VariantList array = mesh.toList();
                auto arrayIt = array.begin();
                int32_t key = arrayIt->toInt();
                ++arrayIt;
                int32_t pageId = arrayIt->toInt();
                ++arrayIt;
                int32_t mode = Complex; // For previous version format compatibility
                if(arrayIt->type() == MetaType::INTEGER) {
                    mode = arrayIt->toInt();
                    ++arrayIt;
                }
                switch(mode) {
                    case Simple: {
                        Vector4 bounds = arrayIt->toVector4();

                        setRegion(key, bounds);
                    } break;
                    case Sliced: {
                        Vector4 bounds(arrayIt->toVector4());
                        ++arrayIt;
                        Vector4 borders(arrayIt->toVector4());
                        ++arrayIt;
                        Vector2 pivot(arrayIt->toVector2());

                        setRegion(key, bounds, borders, pivot);
                    } break;
                    case Complex: {
                        Mesh *m = Engine::loadResource<Mesh>(arrayIt->toString());
                        if(m) {
                            m->incRef();
                            m_shapes[key] = {pageId, mode, m};
                        }
                    } break;
                    default: break;
                }
            }
        }
    }
}
/*!
    \internal
*/
VariantMap Sprite::saveUserData() const {
    VariantMap result;

    VariantList pages;
    for(auto &it : m_pages) {
        TString ref = Engine::reference(it);
        if(!ref.isEmpty()) {
            pages.push_back(ref);
        }
    }
    if(!pages.empty()) {
        result[gPages] = pages;
    }

    VariantList shapes;
    for(auto &it : m_shapes) {
        VariantList fields;

        fields.push_back(it.first);
        fields.push_back(it.second.page);
        fields.push_back(it.second.mode);

        switch(it.second.mode) {
            case Simple: {
                fields.push_back(it.second.bounds);
            } break;
            case Sliced: {
                fields.push_back(it.second.bounds);
                fields.push_back(it.second.border);
                fields.push_back(it.second.pivot);
            } break;
            case Complex: {
                TString ref = Engine::reference(it.second.mesh);
                if(!ref.isEmpty()) {
                    fields.push_back(ref);
                } else {
                    continue;
                }
            } break;
            default: break;
        }

        shapes.push_back(fields);
    }
    if(!shapes.empty()) {
        result[gShapes] = shapes;
    }

    return result;
}
/*!
    Returns a mesh which represents the sprite with \a key.
*/
Mesh *Sprite::shape(int key) const {
    PROFILE_FUNCTION();

    auto it = m_shapes.find(key);
    if(it != m_shapes.end()) {
        return it->second.mesh;
    }
    return nullptr;
}
/*!
    Defines a new sub sprite with \a key.
    Defined sub sprite will be represented as \a mesh.
*/
void Sprite::setShape(int key, Mesh *mesh) {
    PROFILE_FUNCTION();

    if(mesh) {
        mesh->incRef();
        m_shapes[key] = {0, Complex, mesh};
    }
}
/*!
    Returns \a pixels per unit to create sprite meshes.
*/
float Sprite::pixelsPerUnit() const {
    return m_pixelsPerUnit;
}
/*!
    Sets \a pixels per unit to create sprite meshes.
*/
void Sprite::setPixelsPerUnit(float pixels) {
    m_pixelsPerUnit = pixels;
}
/*!
    Defines a new sub sprite with \a key.
    Defined sub sprite will be represented as box with defined \a bounds.
*/
void Sprite::setRegion(int key, const Vector4 &bounds) {
    setRegion(key, bounds, Vector4(), Vector2(0.5f));
}
/*!
    Defines a new 9 patch sub sprite with \a key.
    Defined sub sprite will be represented as box with defined \a bounds in pixels.
    A \a border argument can be used to define border for 9 patch sliced.
    Min corner cotained in x and y components and max corner in z and w.
*/
void Sprite::setRegion(int key, const Vector4 &bounds, const Vector4 &border, const Vector2 &pivot) {
    auto it = m_shapes.find(key);
    if(it != m_shapes.end()) {
        if(it->second.mode != Complex) {
            delete it->second.mesh;
        }
    }

    float width = 1;
    float height = 1;
    if(!m_pages.empty()) {
        width = m_pages.front()->width();
        height = m_pages.front()->height();
    }

    Mesh *mesh = Engine::objectCreate<Mesh>();

    float w = (bounds.z - bounds.x) / m_pixelsPerUnit;
    float h = (bounds.w - bounds.y) / m_pixelsPerUnit;

    float l = border.x / m_pixelsPerUnit;
    float r = border.z / m_pixelsPerUnit;
    float t = border.w / m_pixelsPerUnit;
    float b = border.y / m_pixelsPerUnit;

    mesh->setIndices({0, 1, 5, 0, 5, 4, 1, 2, 6, 1, 6, 5, 2, 3, 7, 2, 7, 6,
                      4, 5, 9, 4, 9, 8, 5, 6,10, 5,10, 9, 6, 7,11, 6,11,10,
                      8, 9,13, 8,13,12, 9,10,14, 9,14,13,10,11,15,10,15,14});

    {
        float x0 = -w * pivot.x;
        float x1 = -w * pivot.x + l;
        float x2 =  w * (1.0f - pivot.x) - r;
        float x3 =  w * (1.0f - pivot.x);

        float y0 = -h * pivot.y;
        float y1 = -h * pivot.y + b;
        float y2 =  h * (1.0f - pivot.y) - t;
        float y3 =  h * (1.0f - pivot.y);

        mesh->setVertices({
            Vector3(x0, y0, 0.0f), Vector3(x1, y0, 0.0f), Vector3(x2, y0, 0.0f), Vector3(x3, y0, 0.0f),
            Vector3(x0, y1, 0.0f), Vector3(x1, y1, 0.0f), Vector3(x2, y1, 0.0f), Vector3(x3, y1, 0.0f),

            Vector3(x0, y2, 0.0f), Vector3(x1, y2, 0.0f), Vector3(x2, y2, 0.0f), Vector3(x3, y2, 0.0f),
            Vector3(x0, y3, 0.0f), Vector3(x1, y3, 0.0f), Vector3(x2, y3, 0.0f), Vector3(x3, y3, 0.0f),
        });
    }
    {
        float x0 = bounds.x / width;
        float x1 = (bounds.x + border[3]) / width;
        float x2 = (bounds.z - border[1]) / width;
        float x3 = bounds.z / width;

        float y0 = bounds.y / height;
        float y1 = (bounds.y + border[2]) / height;
        float y2 = (bounds.w - border[0]) / height;
        float y3 = bounds.w / height;

        mesh->setUv0({
            Vector2(x0, y0), Vector2(x1, y0), Vector2(x2, y0), Vector2(x3, y0),
            Vector2(x0, y1), Vector2(x1, y1), Vector2(x2, y1), Vector2(x3, y1),

            Vector2(x0, y2), Vector2(x1, y2), Vector2(x2, y2), Vector2(x3, y2),
            Vector2(x0, y3), Vector2(x1, y3), Vector2(x2, y3), Vector2(x3, y3),
        });
    }
    mesh->setColors(Vector4Vector(mesh->vertices().size(), Vector4(1.0f)));

    mesh->recalcBounds();

    Shape shape;
    shape.page = 0;
    shape.mode = Sliced;
    shape.mesh = mesh;
    shape.bounds = bounds;
    shape.border = border;
    shape.pivot = pivot;

    m_shapes[key] = shape;
}
/*!
    Returns a sprite sheet texture with \a key.
*/
Texture *Sprite::page(int key) {
    PROFILE_FUNCTION();

    int index = 0;
    auto it = m_shapes.find(key);
    if(it != m_shapes.end()) {
        index = it->second.page;
    }

    return (index < m_pages.size()) ? m_pages[index] : nullptr;
}
/*!
    Adds a new sprite sheet \a texture.
*/
void Sprite::addPage(Texture *texture) {
    PROFILE_FUNCTION();

    if(texture) {
        texture->incRef();
        m_pages.push_back(texture);
    }
}
/*!
    \internal
*/
void Sprite::clear() {
    PROFILE_FUNCTION();

    for(auto it : m_sources) {
        it->decRef();
    }
    m_sources.clear();

    for(auto it : m_pages) {
        it->decRef();
    }
    m_pages.clear();

    for(auto it : m_shapes) {
        it.second.mesh->decRef();
    }
    m_shapes.clear();
}

void composeSliced(Mesh *mesh, Vector2 &size) {
    Vector3Vector &verts = mesh->vertices();

    Vector2 meshSize = mesh->bound().extent * 2;
    Vector2 scl(size.x / meshSize.x, size.y / meshSize.y);

    // horizontal
    float wl = (verts[1].x - verts[0].x);
    float wr = (verts[3].x - verts[2].x);
    float bw = wl + wr; // borders

    float vl = verts[0].x * scl.x;
    float vr = verts[3].x * scl.x;

    verts[ 0].x = vl; verts[ 3].x = vr;
    verts[ 4].x = vl; verts[ 7].x = vr;
    verts[ 8].x = vl; verts[11].x = vr;
    verts[12].x = vl; verts[15].x = vr;

    float dl = verts[0].x + MIN(size.x * (wl / bw), wl);
    float dr = verts[3].x - MIN(size.x * (wr / bw), wr);

    verts[ 1].x = dl; verts[ 2].x = dr;
    verts[ 5].x = dl; verts[ 6].x = dr;
    verts[ 9].x = dl; verts[10].x = dr;
    verts[13].x = dl; verts[14].x = dr;

    // vertical
    float hb = (verts[ 4].y - verts[0].y);
    float ht = (verts[12].y - verts[8].y);
    float bh = hb + ht; // borders

    float vb = verts[ 0].y * scl.y;
    float vt = verts[12].y * scl.y;

    verts[ 0].y = vb; verts[12].y = vt;
    verts[ 1].y = vb; verts[13].y = vt;
    verts[ 2].y = vb; verts[14].y = vt;
    verts[ 3].y = vb; verts[15].y = vt;

    float db = verts[ 0].y + MIN(size.y * (hb / bh), hb);
    float dt = verts[12].y - MIN(size.y * (ht / bh), ht);

    verts[ 4].y = db; verts[ 8].y = dt;
    verts[ 5].y = db; verts[ 9].y = dt;
    verts[ 6].y = db; verts[10].y = dt;
    verts[ 7].y = db; verts[11].y = dt;
}

bool composeTiled(Mesh *mesh, Vector2 &size) {
    Vector3Vector &verts = mesh->vertices();
    IndexVector &indices = mesh->indices();
    Vector2Vector &uvs = mesh->uv0();

    Vector2 meshSize = mesh->bound().extent * 2;

    Vector2 ubl(uvs[0]);
    Vector2 utr(uvs[15]);

    int width = ceilf(size.x / meshSize.x);
    int height = ceilf(size.y / meshSize.y);

    if(width == 0 || height == 0) {
        return false;
    }

    verts.resize(width * height * 4);
    indices.resize(width * height * 6);
    uvs.resize(width * height * 4);

    Vector3 bl(Vector3(size, 0.0f) * -0.5f);

    int i = 0;
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;

            Vector2 f(1.0f);
            if(x == width - 1) {
                f.x = MIN((size.x * 0.5f - bl.x) / meshSize.x, 1.0f);
            }
            if(y == height - 1) {
                f.y = MIN((size.y * 0.5f - bl.y) / meshSize.y, 1.0f);
            }

            verts[index] = bl;
            verts[index + 1] = bl + Vector3(meshSize.x * f.x, 0.0f, 0.0f);
            verts[index + 2] = bl + Vector3(meshSize.x * f.x, meshSize.y * f.y, 0.0f);
            verts[index + 3] = bl + Vector3(0.0f, meshSize.y * f.y, 0.0f);

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

            bl.x += meshSize.x;

            i += 6;
        }
        bl.x = size.x * -0.5f;
        bl.y += meshSize.y;
    }

    mesh->recalcBounds();

    return true;
}
/*!
    \internal
*/
Mesh *Sprite::composeMesh(Mesh *mesh, int key, Mode mode, Vector2 &size) const {
    Mesh *result = shape(key);
    if(result) {
        if(mode == Sliced || mode == Tiled) {
            mesh->setVertices(result->vertices());
            mesh->setIndices(result->indices());
            mesh->setUv0(result->uv0());
            mesh->setBound(result->bound());

            if(mode == Sliced) {
                composeSliced(mesh, size);
            } else if(!composeTiled(mesh, size)) {
                return mesh;
            }

            if(mesh->colors().empty()) {
                mesh->setColors(Vector4Vector(mesh->vertices().size(), Vector4(1.0f)));
            }

            mesh->recalcBounds();

            return mesh;
        }
    }
    return result;
}
