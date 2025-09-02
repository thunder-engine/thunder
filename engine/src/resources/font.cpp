#include "font.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include <freetype/ftbitmap.h>

#include "texture.h"
#include "utils/atlas.h"

#include "log.h"

namespace  {
    const char *gData("Data");
}

#define DF_GLYPH_SIZE 64

struct Point {
    short dx, dy;
    int f;
};

struct Grid {
    int32_t w, h;
    Point *grid;
};

static Point pointInside = { 0, 0, 0 };
static Point pointEmpty = { 9999, 9999, 9999*9999 };

static FT_Library library = nullptr;
//FT_Done_FreeType(library);

static inline Point get(Grid &g, int32_t x, int32_t y) {
    return g.grid[y * (g.w + 2) + x];
}

static inline void put(Grid &g, int32_t x, int32_t y, const Point &p) {
    g.grid[y * (g.w + 2) + x] = p;
}

static inline void compare(Grid &g, Point &p, int32_t x, int32_t y, int32_t offsetx, int32_t offsety) {
    int add;
    Point other = get(g, x + offsetx, y + offsety);
    if(offsety == 0) {
        add = 2 * other.dx + 1;
    } else if(offsetx == 0) {
        add = 2 * other.dy + 1;
    } else {
        add = 2 * (other.dy + other.dx + 1);
    }
    other.f += add;
    if(other.f < p.f) {
        p.f = other.f;
        if(offsety == 0) {
            p.dx = other.dx + 1;
            p.dy = other.dy;
        } else if(offsetx == 0) {
            p.dy = other.dy + 1;
            p.dx = other.dx;
        } else {
            p.dy = other.dy + 1;
            p.dx = other.dx + 1;
        }
    }
}

static void generateSDF(Grid &g) {
    for(int32_t y = 1; y <= g.h; y++) {
        for(int32_t x = 1; x <= g.w; x++) {
            Point p = get(g, x, y);
            compare(g, p, x, y, -1,  0);
            compare(g, p, x, y,  0, -1);
            compare(g, p, x, y, -1, -1);
            compare(g, p, x, y,  1, -1);
            put(g, x, y, p);
        }
    }

    for(int32_t y = g.h; y > 0; y--) {
        for(int32_t x = g.w; x > 0; x--) {
            Point p = get(g, x, y);
            compare(g, p, x, y,  1,  0);
            compare(g, p, x, y,  0,  1);
            compare(g, p, x, y, -1,  1);
            compare(g, p, x, y,  1,  1);
            put(g, x, y, p);
        }
    }
}

/*!
    \class Font
    \brief The Font resource provides support for vector fonts.
    \inmodule Resources

    The basic element of a font is a glyph.
    All required glyphs are contained in a special texture - Atlas.
    If at the moment of accessing the font the glyph is not present in the atlas, the glyph will be loaded there dynamically.
*/

Font::Font() :
        m_face(nullptr),
        m_scale(DF_GLYPH_SIZE),
        m_spaceWidth(0.0f),
        m_lineHeight(0.0f),
        m_useKerning(false) {

    FT_Init_FreeType( &library );
}

Font::~Font() {
     Font::clear();
}
/*!
    Returns the index of the \a glyph in the atlas.
*/
int Font::atlasIndex(int glyph) const {
    auto it = m_glyphMap.find(glyph);
    if(it != m_glyphMap.end()) {
        return (*it).second;
    }
    return 0;
}
/*!
    Requests \a characters to be added to the font atlas.
*/
void Font::requestCharacters(const TString &characters) {
    std::u32string u32 = characters.toUtf32();

    bool isNew = false;
    for(auto it : u32) {
        uint32_t ch = it;
        if(m_glyphMap.find(ch) == m_glyphMap.end() && m_face) {
            FT_Error error = FT_Load_Glyph(reinterpret_cast<FT_FaceRec_ *>(m_face), FT_Get_Char_Index(reinterpret_cast<FT_FaceRec_ *>(m_face), it), FT_LOAD_RENDER);
            if(!error) {
                int index = -1;

                FT_GlyphSlot slot = reinterpret_cast<FT_FaceRec_ *>(m_face)->glyph;
                error = FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);
                if(!error) {
                    if(slot->bitmap.width && slot->bitmap.rows) {
                        Texture::Surface s;
                        ByteArray buffer;
                        buffer.resize(slot->bitmap.width * slot->bitmap.rows);

                        memcpy(buffer.data(), slot->bitmap.buffer, buffer.size());

                        s.push_back(buffer);

                        Texture *t  = Engine::objectCreate<Texture>("", this);
                        t->resize(slot->bitmap.width, slot->bitmap.rows);
                        t->clear();
                        t->addSurface(s);

                        index = addElement(t);
                    }
                }

                FT_Glyph glyph;
                error = FT_Get_Glyph(slot, &glyph);
                if(!error && index > -1) {
                    FT_BBox bbox;
                    FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_pixels, &bbox);

                    Mesh *m = shape(index);
                    if(m) {
                        m->setVertices({Vector3(bbox.xMin, bbox.yMax, 0.0f) / m_scale,
                                        Vector3(bbox.xMax, bbox.yMax, 0.0f) / m_scale,
                                        Vector3(bbox.xMax, bbox.yMin, 0.0f) / m_scale,
                                        Vector3(bbox.xMin, bbox.yMin, 0.0f) / m_scale});
                    }
                    m_glyphMap[ch] = index;

                    isNew = true;
                }
            }
        }
    }

    if(isNew) {
        packSheets(1);
        notifyCurrentState();
    }
}
/*!
    Returns the kerning offset between a \a glyph and \a previous glyph.
    \note In case of font doesn't support kerning this method will return 0.
*/
int Font::requestKerning(int glyph, int previous) const {
    if(m_useKerning && previous)  {
        FT_Vector delta;
        FT_Get_Kerning( reinterpret_cast<FT_FaceRec_ *>(m_face), previous, glyph, FT_KERNING_DEFAULT, &delta );
        return delta.x >> 6;
    }
    return 0;
}
/*!
    Returns the number of \a characters in the string.
*/
int Font::length(const TString &characters) const {
    return characters.toUtf32().length();
}
/*!
    Returns visual width of space character for the font in world units.
*/
float Font::spaceWidth() const {
    return m_spaceWidth;
}
/*!
    Returns visual height for the font in world units.
*/
float Font::lineHeight() const {
    return m_lineHeight;
}

float Font::textWidth(const TString &text, int size, bool kerning) {
    TString data = Engine::translate(text);
    requestCharacters(data);

    float pos = 0;

    uint32_t length = Font::length(data);
    if(length) {
        std::u32string u32 = data.toUtf32();

        uint32_t previous = 0;
        uint32_t it = 0;

        for(uint32_t i = 0; i < length; i++) {
            uint32_t ch = u32[i];
            switch(ch) {
                case ' ': {
                    pos += m_spaceWidth * size;
                } break;
                case '\t': {
                    pos += m_spaceWidth * size * 4;
                } break;
                default: {
                    if(kerning) {
                        pos += requestKerning(ch, previous);
                    }
                    uint32_t index = atlasIndex(ch);

                    Mesh *glyph = shape(index);
                    if(glyph == nullptr) {
                        continue;
                    }
                    Vector3Vector &shape = glyph->vertices();

                    pos += shape[2].x * size;
                    it++;
                } break;
            }
            previous = ch;
        }
    }

    return pos;
}

void Font::composeMesh(Mesh *mesh, const TString &text, int size, int alignment, bool kerning, bool wrap, const Vector2 &boundaries) {
    float spaceWidth = m_spaceWidth * size;
    float spaceLine = m_lineHeight * size;

    TString data = Engine::translate(text);
    requestCharacters(data);

    uint32_t length = Font::length(data);
    if(length) {
        std::u32string u32 = data.toUtf32();

        IndexVector &indices = mesh->indices();
        Vector3Vector &vertices = mesh->vertices();
        Vector2Vector &uv0 = mesh->uv0();
        Vector4Vector &colors = mesh->colors();

        vertices.resize(length * 4);
        indices.resize(length * 6);
        uv0.resize(vertices.size());
        colors.resize(vertices.size());

        std::list<float> width;
        std::list<uint32_t> position;

        Vector3 pos(0.0, boundaries.y - size, 0.0f);
        uint32_t previous = 0;
        uint32_t it = 0;
        uint32_t space = 0;

        for(uint32_t i = 0; i < length; i++) {
            uint32_t ch = u32[i];
            switch(ch) {
                case ' ': {
                    pos += Vector3(spaceWidth, 0.0f, 0.0f);
                    space = it;
                } break;
                case '\t': {
                    pos += Vector3(spaceWidth * 4, 0.0f, 0.0f);
                    space = it;
                } break;
                case '\r': break;
                case '\n': {
                    width.push_back(pos.x);
                    position.push_back(it);
                    pos = Vector3(0.0f, pos.y - spaceLine, 0.0f);
                    space = 0;
                } break;
                default: {
                    if(kerning) {
                        pos.x += requestKerning(ch, previous);
                    }
                    uint32_t index = atlasIndex(ch);

                    Mesh *glyph = shape(index);
                    if(glyph == nullptr) {
                        continue;
                    }

                    Vector3Vector &shape = glyph->vertices();
                    Vector2Vector &uv = glyph->uv0();

                    float x = pos.x + shape[2].x * size;
                    if(wrap && boundaries.x > 0.0f && boundaries.x < x && space > 0 && space < it) {
                        float shift = vertices[space * 4].x;
                        if((shift - spaceWidth) > 0.0f) {
                            for(uint32_t s = space; s < it; s++) {
                                vertices[s * 4 + 0] -= Vector3(shift, spaceLine, 0.0f);
                                vertices[s * 4 + 1] -= Vector3(shift, spaceLine, 0.0f);
                                vertices[s * 4 + 2] -= Vector3(shift, spaceLine, 0.0f);
                                vertices[s * 4 + 3] -= Vector3(shift, spaceLine, 0.0f);
                            }
                            width.push_back(shift - spaceWidth);
                            position.push_back(space);
                            pos = Vector3(pos.x - shift, pos.y - spaceLine, 0.0f);
                        }
                    }

                    vertices[it * 4 + 0] = pos + shape[0] * size;
                    vertices[it * 4 + 1] = pos + shape[1] * size;
                    vertices[it * 4 + 2] = pos + shape[2] * size;
                    vertices[it * 4 + 3] = pos + shape[3] * size;

                    uv0[it * 4 + 0] = uv[0];
                    uv0[it * 4 + 1] = uv[1];
                    uv0[it * 4 + 2] = uv[2];
                    uv0[it * 4 + 3] = uv[3];

                    colors[it * 4 + 0] = Vector4(1.0f);
                    colors[it * 4 + 1] = Vector4(1.0f);
                    colors[it * 4 + 2] = Vector4(1.0f);
                    colors[it * 4 + 3] = Vector4(1.0f);

                    indices[it * 6 + 0] = it * 4 + 0;
                    indices[it * 6 + 1] = it * 4 + 1;
                    indices[it * 6 + 2] = it * 4 + 2;

                    indices[it * 6 + 3] = it * 4 + 0;
                    indices[it * 6 + 4] = it * 4 + 2;
                    indices[it * 6 + 5] = it * 4 + 3;

                    pos += Vector3(shape[2].x * size, 0.0f, 0.0f);
                    it++;
                } break;
            }
            previous = ch;
        }

        width.push_back(pos.x);
        position.push_back(it);

        vertices.resize(it * 4);
        indices.resize(it * 6);
        uv0.resize(it * 4);
        colors.resize(it * 4);

        auto w = width.begin();
        auto p = position.begin();
        float shiftX = (!(alignment & Left)) ? (boundaries.x - (*w)) / ((alignment & Center) ? 2 : 1) : 0.0f;
        float shiftY = (!(alignment & Top)) ? (boundaries.y - position.size() * spaceLine) / ((alignment & Middle) ? 2 : 1) : 0.0f;
        for(uint32_t i = 0; i < vertices.size(); i++) {
            if(uint32_t(i / 4) >= *p) {
                w++;
                p++;
                shiftX = (!(alignment & Left)) ? (boundaries.x - (*w)) / ((alignment & Center) ? 2 : 1) : 0.0f;
            }
            vertices[i].x += shiftX;
            vertices[i].y -= shiftY;
        }

        mesh->recalcBounds();
    }
}
/*!
    \internal
*/
void Font::loadUserData(const VariantMap &data) {
    clear();
    {
        auto it = data.find(gData);
        if(it != data.end()) {

            FT_FaceRec_ *face = reinterpret_cast<FT_FaceRec_ *>(m_face);
            m_data = (*it).second.toByteArray();
            FT_Error error = FT_New_Memory_Face(library, reinterpret_cast<const uint8_t *>(&m_data[0]), m_data.size(), 0, &face);
            if(error) {
                Log(Log::ERR) << "Can't load font. System returned error:" << error;
                return;
            }

            m_face = reinterpret_cast<int32_t *>(face);
            error = FT_Set_Char_Size( face, m_scale * 64, 0, 0, 0 );
            if(error) {
                Log(Log::ERR) << "Can't set default font size. System returned error:" << error;
                return;
            }
            m_useKerning = FT_HAS_KERNING( face );

            error = FT_Load_Glyph( face, FT_Get_Char_Index( face, ' ' ), FT_LOAD_DEFAULT );
            if(!error) {
                m_spaceWidth = static_cast<float>(face->glyph->advance.x) / m_scale / 64.0f;
            }

            error = FT_Load_Glyph( face, FT_Get_Char_Index( face, '\n' ), FT_LOAD_DEFAULT );
            if(!error) {
                m_lineHeight = static_cast<float>(face->glyph->metrics.height) / m_scale / 32.0f;
            }
        }
    }
}
/*!
    \internal
*/
VariantMap Font::saveUserData() const {
    VariantMap result;

    result[gData] = m_data;

    return result;
}

/*!
    Cleans up all font data.
*/
void Font::clear() {
    Sprite::clear();

    m_glyphMap.clear();
    FT_Done_Face(reinterpret_cast<FT_FaceRec_ *>(m_face));
}
