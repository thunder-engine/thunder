#include "sprite.h"

#include "engine.h"
#include "texture.h"

namespace  {
    const char *gTexture("Texture");
}

/*!
    \class Sprite
    \brief Represents 2D sprite.
    \inmodule Resources

    Sprites usually used in games to display environment and characters in 2D games.
*/

Sprite::Sprite() :
        m_pivot(Vector2(0.5f)),
        m_texture(nullptr),
        m_mesh(Engine::objectCreate<Mesh>()),
        m_pixelsPerUnit(1.0f),
        m_mode(Complex),
        m_dirtyMesh(true) {

}

Sprite::~Sprite() {
    PROFILE_FUNCTION();

    if(m_texture) {
        m_texture->decRef();
    }
}
/*!
    \internal
*/
void Sprite::loadUserData(const VariantMap &data) {
    if(m_mode == Complex) {
        m_mesh->loadUserData(data);
    }

    auto it = data.find(gTexture);
    if(it != data.end()) {
        setTexture(Engine::loadResource<Texture>(it->second.toString()));
    }
}
/*!
    \internal
*/
VariantMap Sprite::saveUserData() const {
    VariantMap result;

    if(m_mode == Complex) {
        result = m_mesh->saveUserData();
    }

    if(m_texture) {
        result[gTexture] = Engine::reference(m_texture);
    }

    return result;
}
/*!
    Returns a mesh which represents the sprite.
*/
Mesh *Sprite::mesh() const {
    PROFILE_FUNCTION();

    return m_mesh;
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
    Returns bounds of the Sprite.
*/
Vector4 Sprite::bounds() const {
    return m_bounds;
}
/*!
    Sets \a bounds of the Sprite.
*/
void Sprite::setBounds(const Vector4 &bounds) {
    m_bounds = bounds;
    m_dirtyMesh = true;
}
/*!
    Returns the border sizes of the Sprite.

    X=left, Y=bottom, Z=right, W=top.
*/
Vector4 Sprite::border() const {
    return m_border;
}
/*!
    Sets new \a border sizes for the Sprite.
*/
void Sprite::setBorder(const Vector4 &border) {
    m_border = border;
    m_dirtyMesh = true;
}
/*!
    Returns pivot point of the Sprite.
*/
Vector2 Sprite::pivot() const {
    return m_pivot;
}
/*!
    Sets new \a pivot point for the Sprite.
*/
void Sprite::setPivot(const Vector2 &pivot) {
    m_pivot = pivot;
    m_dirtyMesh = true;
}
/*!
    Returns a sprite texture.
*/
Texture *Sprite::texture() const {
    PROFILE_FUNCTION();

    return m_texture;
}
/*!
    Sets a new sprite texture.
*/
void Sprite::setTexture(Texture *texture) {
    PROFILE_FUNCTION();

    m_texture = texture;
    if(m_texture) {
        m_texture->incRef();
    }
}

int Sprite::mode() const {
    return m_mode;
}

void Sprite::setMode(int mode) {
    m_mode = mode;
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
void Sprite::recalcMesh() const {
    float width = 1;
    float height = 1;
    if(m_texture) {
        width = m_texture->width();
        height = m_texture->height();
    }

    float w = (m_bounds.z - m_bounds.x) / m_pixelsPerUnit;
    float h = (m_bounds.w - m_bounds.y) / m_pixelsPerUnit;

    float l = m_border.x / m_pixelsPerUnit;
    float r = m_border.z / m_pixelsPerUnit;
    float t = m_border.w / m_pixelsPerUnit;
    float b = m_border.y / m_pixelsPerUnit;

    m_mesh->setIndices({0, 1, 5, 0, 5, 4, 1, 2, 6, 1, 6, 5, 2, 3, 7, 2, 7, 6,
                        4, 5, 9, 4, 9, 8, 5, 6,10, 5,10, 9, 6, 7,11, 6,11,10,
                        8, 9,13, 8,13,12, 9,10,14, 9,14,13,10,11,15,10,15,14});
    {
        float x0 = -w * m_pivot.x;
        float x1 = -w * m_pivot.x + l;
        float x2 =  w * (1.0f - m_pivot.x) - r;
        float x3 =  w * (1.0f - m_pivot.x);

        float y0 = -h * m_pivot.y;
        float y1 = -h * m_pivot.y + b;
        float y2 =  h * (1.0f - m_pivot.y) - t;
        float y3 =  h * (1.0f - m_pivot.y);

        m_mesh->setVertices({
            Vector3(x0, y0, 0.0f), Vector3(x1, y0, 0.0f), Vector3(x2, y0, 0.0f), Vector3(x3, y0, 0.0f),
            Vector3(x0, y1, 0.0f), Vector3(x1, y1, 0.0f), Vector3(x2, y1, 0.0f), Vector3(x3, y1, 0.0f),

            Vector3(x0, y2, 0.0f), Vector3(x1, y2, 0.0f), Vector3(x2, y2, 0.0f), Vector3(x3, y2, 0.0f),
            Vector3(x0, y3, 0.0f), Vector3(x1, y3, 0.0f), Vector3(x2, y3, 0.0f), Vector3(x3, y3, 0.0f),
        });
    }
    {
        float x0 = m_bounds.x / width;
        float x1 = (m_bounds.x + m_border[3]) / width;
        float x2 = (m_bounds.z - m_border[1]) / width;
        float x3 = m_bounds.z / width;

        float y0 = m_bounds.y / height;
        float y1 = (m_bounds.y + m_border[2]) / height;
        float y2 = (m_bounds.w - m_border[0]) / height;
        float y3 = m_bounds.w / height;

        m_mesh->setUv0({
            Vector2(x0, y0), Vector2(x1, y0), Vector2(x2, y0), Vector2(x3, y0),
            Vector2(x0, y1), Vector2(x1, y1), Vector2(x2, y1), Vector2(x3, y1),

            Vector2(x0, y2), Vector2(x1, y2), Vector2(x2, y2), Vector2(x3, y2),
            Vector2(x0, y3), Vector2(x1, y3), Vector2(x2, y3), Vector2(x3, y3),
        });
    }
    m_mesh->setColors(Vector4Vector(m_mesh->vertices().size(), Vector4(1.0f)));

    m_mesh->recalcBounds();
}
/*!
    \internal
*/
Mesh *Sprite::composeMesh(Mesh *mesh, Mode mode, Vector2 &size) {
    if(m_mesh) {
        if(m_dirtyMesh && m_mode != Complex) {
            recalcMesh();
            m_dirtyMesh = false;
        }

        if(mode == Sliced || mode == Tiled) {
            mesh->setVertices(m_mesh->vertices());
            mesh->setIndices(m_mesh->indices());
            mesh->setUv0(m_mesh->uv0());
            mesh->setBound(m_mesh->bound());

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
    return m_mesh;
}
