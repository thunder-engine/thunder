#include "font.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include <freetype/ftbitmap.h>

#include "texture.h"
#include "utils/atlas.h"
#include "utils.h"

#include "log.h"

namespace  {
    const char *gHeader = "Header";
    const char *gData = "Data";
}

#define DF_GLYPH_SIZE 64

class FontPrivate {
public:
    FontPrivate() :
            m_face(nullptr),
            m_scale(DF_GLYPH_SIZE),
            m_spaceWidth(0.0f),
            m_lineHeight(0.0f),
            m_cursorWidth(0.0f),
            m_useKerning(false) {
    }

    typedef unordered_map<uint32_t, uint32_t> GlyphMap;
    typedef unordered_map<uint32_t, Vector2> SpecialMap;

    GlyphMap m_glyphMap;
    ByteArray m_data;

    FT_FaceRec_ *m_face;

    int32_t m_scale;

    float m_spaceWidth;
    float m_lineHeight;
    float m_cursorWidth;

    bool m_useKerning;

};

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
        p_ptr(new FontPrivate()) {

    FT_Init_FreeType( &library );
}

Font::~Font() {
     Font::clear();
}
/*!
    Returns the index of the \a glyph in the atlas.
*/
int Font::atlasIndex(int glyph) const {
    auto it = p_ptr->m_glyphMap.find(glyph);
    if(it != p_ptr->m_glyphMap.end()) {
        return (*it).second;
    }
    return 0;
}
/*!
    Requests \a characters to be added to the font atlas.
*/
void Font::requestCharacters(const string &characters) {
    u32string u32 = Utils::utf8ToUtf32(characters);

    bool isNew = false;
    for(auto it : u32) {
        uint32_t ch = it;
        if(p_ptr->m_glyphMap.find(ch) == p_ptr->m_glyphMap.end() && p_ptr->m_face) {
            FT_Error error = FT_Load_Glyph(p_ptr->m_face, FT_Get_Char_Index(p_ptr->m_face, it), FT_LOAD_RENDER);
            if(!error) {
                int index = -1;

                FT_GlyphSlot slot = p_ptr->m_face->glyph;
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
                        m->setVertices({Vector3(bbox.xMin, bbox.yMax, 0.0f) / p_ptr->m_scale,
                                        Vector3(bbox.xMax, bbox.yMax, 0.0f) / p_ptr->m_scale,
                                        Vector3(bbox.xMax, bbox.yMin, 0.0f) / p_ptr->m_scale,
                                        Vector3(bbox.xMin, bbox.yMin, 0.0f) / p_ptr->m_scale});
                    }
                    p_ptr->m_glyphMap[ch] = index;

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
    if(p_ptr->m_useKerning && previous)  {
        FT_Vector delta;
        FT_Get_Kerning( p_ptr->m_face, previous, glyph, FT_KERNING_DEFAULT, &delta );
        return delta.x >> 6;
    }
    return 0;
}
/*!
    Returns the number of \a characters in the string.
*/
int Font::length(const string &characters) const {
    u32string u32 = Utils::utf8ToUtf32(characters);
    return u32.length();
}
/*!
    Returns visual width of space character for the font in world units.
*/
float Font::spaceWidth() const {
    return p_ptr->m_spaceWidth;
}
/*!
    Returns visual height for the font in world units.
*/
float Font::lineHeight() const {
    return p_ptr->m_lineHeight;
}
/*!
    Returns visual width of the cursor for the font in world units.
*/
float Font::cursorWidth() const {
    return p_ptr->m_cursorWidth;
}
/*!
    \internal
*/
void Font::loadUserData(const VariantMap &data) {
    clear();
    {
        auto it = data.find(gData);
        if(it != data.end()) {
            p_ptr->m_data = (*it).second.toByteArray();
            FT_Error error = FT_New_Memory_Face(library, reinterpret_cast<const uint8_t *>(&p_ptr->m_data[0]), p_ptr->m_data.size(), 0, &p_ptr->m_face);
            if(error) {
                Log(Log::ERR) << "Can't load font. System returned error:" << error;
                return;
            }
            error = FT_Set_Char_Size( p_ptr->m_face, p_ptr->m_scale * 64, 0, 0, 0 );
            if(error) {
                Log(Log::ERR) << "Can't set default font size. System returned error:" << error;
                return;
            }
            p_ptr->m_useKerning = FT_HAS_KERNING( p_ptr->m_face );

            error = FT_Load_Glyph( p_ptr->m_face, FT_Get_Char_Index( p_ptr->m_face, ' ' ), FT_LOAD_DEFAULT );
            if(!error) {
                p_ptr->m_spaceWidth = static_cast<float>(p_ptr->m_face->glyph->advance.x) / p_ptr->m_scale / 64.0f;
            }

            error = FT_Load_Glyph( p_ptr->m_face, FT_Get_Char_Index( p_ptr->m_face, '\n' ), FT_LOAD_DEFAULT );
            if(!error) {
                p_ptr->m_lineHeight = static_cast<float>(p_ptr->m_face->glyph->metrics.height) / p_ptr->m_scale / 32.0f;
            }

            error = FT_Load_Glyph( p_ptr->m_face, FT_Get_Char_Index( p_ptr->m_face, '|' ), FT_LOAD_DEFAULT );
            if(!error) {
                p_ptr->m_cursorWidth = static_cast<float>(p_ptr->m_face->glyph->advance.x) / p_ptr->m_scale / 64.0f;
            }

        }
    }
}
/*!
    \internal
*/
VariantMap Font::saveUserData() const {
    VariantMap result;

    VariantList header;
    header.push_back(0); // Reserved
    header.push_back(0);
    header.push_back("");

    result[gHeader] = header;
    result[gData] = p_ptr->m_data;

    return result;
}

/*!
    Cleans up all font data.
*/
void Font::clear() {
    Sprite::clear();

    p_ptr->m_glyphMap.clear();
    FT_Done_Face(p_ptr->m_face);
}
