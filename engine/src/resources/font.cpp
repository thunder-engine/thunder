#include "font.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include <freetype/ftbitmap.h>

#include "texture.h"
#include "mesh.h"
#include "utils/atlas.h"

#include "log.h"

namespace  {
    const char *gData("Data");
}

#define DF_GLYPH_SIZE 64

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
        m_page(nullptr),
        m_root(new AtlasNode),
        m_useKerning(false) {

}

Font::~Font() {
    clear();
}
/*!
    \internal
*/
void Font::requestCharacters(const std::u32string &characters, uint32_t size) {
    FT_Face face = reinterpret_cast<FT_Face>(m_face);
    if(face == nullptr) {
        return;
    }

    FT_Error error = FT_Set_Pixel_Sizes(face, 0, size);
    if(error != 0) {
        return;
    }

    bool isNew = false;
    for(auto it : characters) {
        uint32_t ch = it;
        Mathf::hashCombine(ch, size);
        if(m_shapes.find(ch) == m_shapes.end()) {
            error = FT_Load_Glyph(face, FT_Get_Char_Index(face, it), FT_LOAD_RENDER);
            if(!error) {
                FT_GlyphSlot slot = face->glyph;
                error = FT_Render_Glyph(slot, (size < DF_GLYPH_SIZE) ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_SDF);
                if(!error) {
                    if(slot->bitmap.width && slot->bitmap.rows) {
                        GlyphData data;

                        FT_Glyph ftGlyph;
                        error = FT_Get_Glyph(slot, &ftGlyph);
                        if(!error) {
                            FT_BBox bbox;
                            FT_Glyph_Get_CBox(ftGlyph, ft_glyph_bbox_pixels, &bbox);

                            data.vertices = {Vector3(bbox.xMin, bbox.yMax, 0.0f) / static_cast<float>(size),
                                             Vector3(bbox.xMax, bbox.yMax, 0.0f) / static_cast<float>(size),
                                             Vector3(bbox.xMax, bbox.yMin, 0.0f) / static_cast<float>(size),
                                             Vector3(bbox.xMin, bbox.yMin, 0.0f) / static_cast<float>(size)};

                            data.indices = {0, 1, 2, 0, 2, 3};

                            isNew = true;
                        }

                        data.width = slot->bitmap.width;
                        data.height = slot->bitmap.rows;
                        data.data.resize(data.width * data.height);
                        memcpy(data.data.data(), slot->bitmap.buffer, data.data.size());

                        m_shapes[ch] = data;
                    }
                }
            }
        }
    }

    if(isNew) {
        packSheets(10);
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
        FT_Get_Kerning( reinterpret_cast<FT_Face>(m_face), previous, glyph, FT_KERNING_DEFAULT, &delta );
        return delta.x >> 6;
    }
    return 0;
}

float Font::textWidth(const TString &text, int size, int flags) {
    float pos = 0;

    std::u32string u32 = text.toUtf32();
    uint32_t length = u32.length();
    if(length) {
        int adjustedSize = (flags & Sdf) ? DF_GLYPH_SIZE : size;
        requestCharacters(u32, adjustedSize);

        FT_Face face = reinterpret_cast<FT_Face>(m_face);

        uint32_t previous = 0;
        uint32_t it = 0;

        float spaceWidth = 0;
        FT_Error error = FT_Load_Glyph( face, FT_Get_Char_Index( face, ' ' ), FT_LOAD_BITMAP_METRICS_ONLY );
        if(!error) {
            spaceWidth = (adjustedSize == DF_GLYPH_SIZE) ? (DF_GLYPH_SIZE * 64.0f * size) : 64.0f;
            spaceWidth = face->glyph->advance.x / spaceWidth;
        }

        for(uint32_t i = 0; i < length; i++) {
            uint32_t ch = u32[i];
            Mathf::hashCombine(ch, adjustedSize);
            switch(ch) {
                case ' ': {
                    pos += spaceWidth;
                } break;
                case '\t': {
                    pos += spaceWidth * 4;
                } break;
                default: {
                    if(flags & Kerning) {
                        pos += requestKerning(ch, previous);
                    }

                    GlyphData *data = glyph(ch);
                    if(data == nullptr) {
                        continue;
                    }
                    Vector3Vector &shape = data->vertices;

                    pos += shape[2].x * size;
                    it++;
                } break;
            }
            previous = ch;
        }
    }

    return pos;
}

void Font::composeMesh(Mesh *mesh, const TString &text, const Settings &settings) {
    std::u32string u32 = text.toUtf32();
    uint32_t length = u32.length();
    if(length) {
        int adjustedSize = (settings.flags & Sdf) ? DF_GLYPH_SIZE : settings.size;
        requestCharacters(u32, adjustedSize);

        FT_Face face = reinterpret_cast<FT_Face>(m_face);

        float spaceWidth = 0;
        float spaceLine = settings.size * 1.2f;

        FT_Error error = FT_Load_Glyph( face, FT_Get_Char_Index( face, ' ' ), FT_LOAD_BITMAP_METRICS_ONLY );
        if(!error) {
            spaceWidth = face->glyph->advance.x / 64.0f;
        }

        IndexVector &indices = mesh->indices();
        Vector3Vector &vertices = mesh->vertices();
        Vector2Vector &uv0 = mesh->uv0();
        Vector4Vector &colors = mesh->colors();

        uint32_t begin = 0;
        uint32_t vertexCount = length * 4;
        uint32_t indexCount = length * 6;
        if(settings.flags & Additive) {
            vertexCount += vertices.size();
            indexCount += indices.size();
            begin = vertices.size() / 4;
        }

        vertices.resize(vertexCount);
        indices.resize(indexCount);
        uv0.resize(vertexCount);
        colors.resize(vertexCount);

        std::list<float> width;
        std::list<uint32_t> position;

        Vector3 pos(settings.offset.x, settings.boundaries.y - settings.size - settings.offset.y, 0.0f);
        uint32_t previous = 0;
        uint32_t index = begin;
        uint32_t space = index;

        for(uint32_t i = 0; i < length; i++) {
            uint32_t ch = u32[i];
            switch(ch) {
                case ' ': {
                    pos.x += spaceWidth;
                    space = index;
                } break;
                case '\t': {
                    pos.x += spaceWidth * 4;
                    space = index;
                } break;
                case '\r': break;
                case '\n': {
                    width.push_back(pos.x);
                    position.push_back(index);
                    pos.y -= spaceLine;
                    space = 0;
                } break;
                default: {
                    if(settings.flags & Kerning) {
                        pos.x += requestKerning(ch, previous);
                        previous = ch;
                    }

                    Mathf::hashCombine(ch, adjustedSize);
                    GlyphData *data = glyph(ch);
                    if(data == nullptr) {
                        continue;
                    }

                    const Vector3Vector &shape = data->vertices;
                    const Vector2Vector &uv = data->uvs;

                    float x = pos.x + shape[2].x * settings.size;
                    if((settings.flags & Wrap) && settings.boundaries.x > 0.0f && settings.boundaries.x < x && space > 0 && space < index) {
                        float shift = vertices[space * 4].x;
                        if((shift - spaceWidth) > 0.0f) {
                            for(uint32_t s = space; s < index; s++) {
                                vertices[s * 4 + 0] -= Vector3(shift, spaceLine, 0.0f);
                                vertices[s * 4 + 1] -= Vector3(shift, spaceLine, 0.0f);
                                vertices[s * 4 + 2] -= Vector3(shift, spaceLine, 0.0f);
                                vertices[s * 4 + 3] -= Vector3(shift, spaceLine, 0.0f);
                            }
                            width.push_back(shift - spaceWidth);
                            position.push_back(space);
                            pos.x -= shift;
                            pos.y -= spaceLine;
                        }
                    }

                    vertices[index * 4 + 0] = pos + shape[0] * settings.size;
                    vertices[index * 4 + 1] = pos + shape[1] * settings.size;
                    vertices[index * 4 + 2] = pos + shape[2] * settings.size;
                    vertices[index * 4 + 3] = pos + shape[3] * settings.size;

                    uv0[index * 4 + 0] = uv[0];
                    uv0[index * 4 + 1] = uv[1];
                    uv0[index * 4 + 2] = uv[2];
                    uv0[index * 4 + 3] = uv[3];

                    colors[index * 4 + 0] = settings.color;
                    colors[index * 4 + 1] = settings.color;
                    colors[index * 4 + 2] = settings.color;
                    colors[index * 4 + 3] = settings.color;

                    indices[index * 6 + 0] = index * 4 + 0;
                    indices[index * 6 + 1] = index * 4 + 1;
                    indices[index * 6 + 2] = index * 4 + 2;

                    indices[index * 6 + 3] = index * 4 + 0;
                    indices[index * 6 + 4] = index * 4 + 2;
                    indices[index * 6 + 5] = index * 4 + 3;

                    pos.x += shape[2].x * settings.size;
                    index++;
                } break;
            }

        }

        width.push_back(pos.x);
        position.push_back(index);

        vertices.resize(index * 4);
        indices.resize(index * 6);
        uv0.resize(index * 4);
        colors.resize(index * 4);

        auto w = width.begin();
        auto p = position.begin();
        float shiftX = 0.0f;
        if(!(settings.alignment & Left)) {
            shiftX = (settings.boundaries.x - (*w)) / ((settings.alignment & Center) ? 2 : 1);
        }
        float shiftY = 0.0f;
        if(!(settings.alignment & Top)) {
            shiftY = (settings.boundaries.y - position.size() * spaceLine) / ((settings.alignment & Middle) ? 2 : 1);
        }
        for(uint32_t i = 0; i < index - begin; i++) {
            if(i >= *p) {
                w++;
                p++;
                if(!(settings.alignment & Left)) {
                    shiftX = (settings.boundaries.x - (*w)) / ((settings.alignment & Center) ? 2 : 1);
                }
            }
            int32_t index = (i + begin) * 4;
            vertices[index + 0].x += shiftX;
            vertices[index + 0].y -= shiftY;

            vertices[index + 1].x += shiftX;
            vertices[index + 1].y -= shiftY;

            vertices[index + 2].x += shiftX;
            vertices[index + 2].y -= shiftY;

            vertices[index + 3].x += shiftX;
            vertices[index + 3].y -= shiftY;
        }

        mesh->recalcBounds();
    }
}
/*!
    \internal
*/
void Font::loadUserData(const VariantMap &data) {
    clear();

    static FT_Library library = nullptr;
    if(library == nullptr) {
        FT_Init_FreeType( &library );
    }

    auto it = data.find(gData);
    if(it != data.end()) {
        FT_Face face = reinterpret_cast<FT_Face>(m_face);
        m_data = (*it).second.toByteArray();
        FT_Error error = FT_New_Memory_Face(library, m_data.data(), m_data.size(), 0, &face);
        if(error) {
            Log(Log::ERR) << "Can't load font. System returned error:" << error;
            return;
        }
        m_useKerning = FT_HAS_KERNING( face );
        m_face = reinterpret_cast<int32_t *>(face);
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
    FT_Done_Face(reinterpret_cast<FT_FaceRec_ *>(m_face));

    if(m_page) {
        m_page->decRef();
    }
    m_page = nullptr;

    m_shapes.clear();
}
/*!
    \internal
*/
void Font::clearAtlas() {
    if(m_root->left) {
        delete m_root->left;
        m_root->left = nullptr;
    }

    if(m_root->right) {
        delete m_root->right;
        m_root->right = nullptr;
    }

    m_root->occupied = false;
}
/*!
    \internal
*/
Font::GlyphData *Font::glyph(int key) {
    PROFILE_FUNCTION();

    auto it = m_shapes.find(key);
    if(it != m_shapes.end()) {
        return &(it->second);
    }
    return nullptr;
}
/*!
    Packs all added elements into a glyph sheets.
    Parameter \a padding can be used to delimit elements.
*/
void Font::packSheets(int padding) {
    PROFILE_FUNCTION();

    uint32_t atlasWidth = 1024;
    uint32_t atlasHeight = 1024;

    while(true) {
        m_root->w = atlasWidth;
        m_root->h = atlasHeight;

        uint32_t i = 0;
        for(auto &it : m_shapes) {
            i++;
            if(it.second.node) {
                continue;
            }
            int32_t width = (it.second.width + padding * 2);
            int32_t height = (it.second.height + padding * 2);

            AtlasNode *node = m_root->insert(width, height);
            if(node) {
                node->occupied = true;
                it.second.node = node;
            } else { // Not enough space. Increasing page size
                atlasWidth *= 2;
                atlasHeight *= 2;

                clearAtlas();

                for(auto &it : m_shapes) {
                    it.second.copied = false;
                    it.second.node = nullptr;
                }

                break;
            }
        }

        if(i == m_shapes.size()) {
            break;
        }
    }

    Texture *page = Font::page();
    if(page) {
        page->resize(atlasWidth, atlasHeight);
        for(auto &it : m_shapes) {
            if(!it.second.copied) {
                AtlasNode *node = it.second.node;
                uint8_t *src = it.second.data.data();
                uint8_t *dst = page->surface(0).front().data();
                for(int32_t y = 0; y < it.second.height; y++) {
                    uint32_t index = (node->y + y + padding) * atlasWidth + node->x + padding;
                    memcpy(&dst[index], &src[y * it.second.width], it.second.width);
                }

                it.second.copied = true;

                Vector4 uvFrame;
                uvFrame.x = (node->x + padding) / static_cast<float>(atlasWidth);
                uvFrame.y = (node->y + padding) / static_cast<float>(atlasHeight);
                uvFrame.z = uvFrame.x + it.second.width / static_cast<float>(atlasWidth);
                uvFrame.w = uvFrame.y + it.second.height / static_cast<float>(atlasHeight);

                it.second.uvs = {Vector2(uvFrame.x, uvFrame.y),
                                 Vector2(uvFrame.z, uvFrame.y),
                                 Vector2(uvFrame.z, uvFrame.w),
                                 Vector2(uvFrame.x, uvFrame.w)};
            }
        }

        page->setDirty();
    }
}
/*!
    Returns glyph sheet texture.
*/
Texture *Font::page() {
    PROFILE_FUNCTION();

    if(m_page == nullptr) {
        m_page = Engine::objectCreate<Texture>();
        m_page->incRef();
        m_page->setFiltering(Texture::None);
    }

    return m_page;
}
