/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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

#include <cstddef>

namespace  {
const char *gData("Data");

const int gSdfGlyphSize = 64;
}

bool copyBitmap(const FT_Bitmap &bitmap, ByteArray &data) {
    uint32_t bytesPerRow = 0;
    switch(bitmap.pixel_mode) {
    case FT_PIXEL_MODE_GRAY:
        bytesPerRow = bitmap.width;
        if(bitmap.num_grays < 2) {
            return false;
        }
        break;
    case FT_PIXEL_MODE_MONO:
        bytesPerRow = (bitmap.width + 7) / 8;
        break;
    case FT_PIXEL_MODE_GRAY2:
        bytesPerRow = (bitmap.width + 3) / 4;
        break;
    case FT_PIXEL_MODE_GRAY4:
        bytesPerRow = (bitmap.width + 1) / 2;
        break;
    case FT_PIXEL_MODE_BGRA:
        bytesPerRow = bitmap.width * 4;
        break;
    default:
        return false;
    }

    const int pitch = bitmap.pitch >= 0 ? bitmap.pitch : -bitmap.pitch;
    if(!bitmap.buffer || static_cast<uint32_t>(pitch) < bytesPerRow) {
        return false;
    }

    data.resize(static_cast<size_t>(bitmap.width) * bitmap.rows);
    for(uint32_t y = 0; y < bitmap.rows; ++y) {
        // Bitmaps with negative pitch are stored bottom-up.
        const uint8_t *row = bitmap.pitch >= 0
                                 ? bitmap.buffer + static_cast<ptrdiff_t>(y) * bitmap.pitch
                                 : bitmap.buffer + static_cast<ptrdiff_t>(bitmap.rows - 1 - y) * (-bitmap.pitch);
        for(uint32_t x = 0; x < bitmap.width; ++x) {
            uint8_t value = 0;
            switch(bitmap.pixel_mode) {
            case FT_PIXEL_MODE_GRAY:
                value = row[x];
                if(bitmap.num_grays != 256) {
                    value = static_cast<uint8_t>(value * 255 / (bitmap.num_grays - 1));
                }
                break;
            case FT_PIXEL_MODE_MONO:
                value = (row[x >> 3] & (0x80 >> (x & 7))) ? 255 : 0;
                break;
            case FT_PIXEL_MODE_GRAY2:
                value = static_cast<uint8_t>(((row[x >> 2] >> ((3 - (x & 3)) * 2)) & 0x03) * 85);
                break;
            case FT_PIXEL_MODE_GRAY4:
                value = static_cast<uint8_t>(((row[x >> 1] >> ((1 - (x & 1)) * 4)) & 0x0f) * 17);
                break;
            case FT_PIXEL_MODE_BGRA:
                value = row[x * 4 + 3];
                break;
            default:
                return false;
            }
            data[static_cast<size_t>(y) * bitmap.width + x] = value;
        }
    }
    return true;
}

uint32_t glyphKey(uint32_t ch, uint32_t size) {
    uint32_t key = ch;
    Mathf::hashCombine(key, size);
    return key;
}

/*!
    \class Font
    \brief The Font resource provides support for vector fonts.
    \inmodule Resources

    The basic element of a font is a glyph.
    All required glyphs are contained in a special texture - Atlas.
    If at the moment of accessing the font the glyph is not present in the atlas, the glyph will be loaded there dynamically.

    Bitmap and SDF glyphs are kept in two independent atlases, because they
    differ in filtering, pixel meaning and packing strategy.

    The Sdf flag is a hard contract: when it is set, glyphs are always baked
    as SDF at gSdfGlyphSize, regardless of the requested text size. The caller
    is responsible for picking a shader that matches the selected atlas.
*/

Font::Font() :
    m_face(nullptr),
    m_hasKerning(false) {

    m_bitmapAtlas.root = new AtlasNode;
    m_bitmapAtlas.sdf = false;
    m_bitmapAtlas.padding = 2;

    m_sdfAtlas.root = new AtlasNode;
    m_sdfAtlas.sdf = true;
    m_sdfAtlas.padding = 4;
}

Font::~Font() {
    clear();
}
/*!
    \internal
*/
void Font::requestCharacters(const std::u32string &characters, uint32_t size, int flags) {
    FT_Face face = reinterpret_cast<FT_Face>(m_face);
    if(face == nullptr) {
        return;
    }

    // Sdf is a hard contract: it selects both the atlas and the bake mode.
    const bool sdf = (flags & Sdf) != 0;
    const int adjustedSize = sdf ? gSdfGlyphSize : static_cast<int>(size);

    FT_Error error = FT_Set_Pixel_Sizes(face, 0, adjustedSize);
    if(error != 0) {
        return;
    }

    GlyphAtlas &atlas = atlasFor(flags);

    for(auto it : characters) {
        uint32_t ch = it;
        if(atlas.shapes.find(glyphKey(ch, adjustedSize)) != atlas.shapes.end()) {
            continue;
        }

        const FT_UInt index = FT_Get_Char_Index(face, it);
        if(index == 0) {
            // Skip missing glyphs instead of silently baking .notdef.
            continue;
        }

        FT_Int32 loadFlags = FT_LOAD_DEFAULT;
        if(sdf) {
            // SDF must store true outline distances, so hinting and embedded
            // bitmaps would corrupt the distance field.
            loadFlags = FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING;
        } else if(adjustedSize < 16) {
            // Light hinting keeps small text crisp without thickening stems.
            loadFlags |= FT_LOAD_TARGET_LIGHT;
        }

        error = FT_Load_Glyph(face, index, loadFlags);
        if(error) {
            continue;
        }

        FT_GlyphSlot slot = face->glyph;
        if(slot->format != FT_GLYPH_FORMAT_BITMAP) {
            const FT_Render_Mode mode = sdf ? FT_RENDER_MODE_SDF : FT_RENDER_MODE_NORMAL;
            error = FT_Render_Glyph(slot, mode);
        }
        if(error) {
            continue;
        }

        if(slot->bitmap.width == 0 || slot->bitmap.rows == 0) {
            continue;
        }

        GlyphData data;
        data.width = slot->bitmap.width;
        data.height = slot->bitmap.rows;

        // Vertices are normalized by the baked size so that they can be
        // scaled to any target size at draw time.
        const float inverseSize = 1.0f / static_cast<float>(adjustedSize);
        const float left = static_cast<float>(slot->bitmap_left);
        const float top = static_cast<float>(slot->bitmap_top);
        const float right = left + slot->bitmap.width;
        const float bottom = top - slot->bitmap.rows;

        data.vertices = {Vector3(left, top, 0.0f) * inverseSize,
                         Vector3(right, top, 0.0f) * inverseSize,
                         Vector3(right, bottom, 0.0f) * inverseSize,
                         Vector3(left, bottom, 0.0f) * inverseSize};
        data.indices = {0, 1, 2, 0, 2, 3};

        if(copyBitmap(slot->bitmap, data.data)) {
            atlas.shapes[glyphKey(ch, adjustedSize)] = data;
            atlas.dirty = true;
        } else {
            Log(Log::ERR) << "Unable to copy font glyph bitmap:" << static_cast<uint32_t>(it);
        }
    }
}
/*!
    Returns the kerning offset between a \a glyph and \a previous glyph.
    \note In case of font doesn't support kerning this method will return 0.
*/
int Font::requestKerning(int glyph, int previous, bool sdf) const {
    if(m_hasKerning && previous)  {
        FT_Vector delta;
        // SDF glyphs are not hinted, so kerning must be unfitted as well.
        const FT_UInt kerningMode = sdf ? FT_KERNING_UNFITTED : FT_KERNING_DEFAULT;
        FT_Get_Kerning(reinterpret_cast<FT_Face>(m_face), previous, glyph, kerningMode, &delta);
        return delta.x >> 6;
    }
    return 0;
}

float Font::textWidth(const TString &text, int size, int flags) {
    float pos = 0;

    std::u32string u32 = text.toUtf32();
    uint32_t length = u32.length();
    if(length) {
        const bool sdf = (flags & Sdf) != 0;
        const int adjustedSize = sdf ? gSdfGlyphSize : size;
        requestCharacters(u32, adjustedSize, flags);

        FT_Face face = reinterpret_cast<FT_Face>(m_face);

        uint32_t previous = 0;

        // Space advance is taken from the baked size, then scaled to the
        // requested size. For SDF this is a real scale factor, not 1.
        float spaceWidth = 0;
        FT_Error error = FT_Load_Glyph(face, FT_Get_Char_Index(face, ' '), FT_LOAD_BITMAP_METRICS_ONLY);
        if(!error) {
            spaceWidth = (face->glyph->advance.x / 64.0f) * (static_cast<float>(size) / static_cast<float>(adjustedSize));
        }

        for(uint32_t i = 0; i < length; i++) {
            uint32_t ch = u32[i];
            switch(ch) {
                case ' ': {
                    pos += spaceWidth;
                } break;
                case '\t': {
                    pos += spaceWidth * 4;
                } break;
                default: {
                    if(flags & Kerning) {
                        pos += requestKerning(ch, previous, sdf);
                    }

                    GlyphData *data = glyph(glyphKey(ch, adjustedSize), flags);
                    if(data == nullptr) {
                        continue;
                    }
                    Vector3Vector &shape = data->vertices;

                    pos += shape[2].x * size;
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
        const bool sdf = (settings.flags & Sdf) != 0;
        const int adjustedSize = sdf ? gSdfGlyphSize : settings.size;
        requestCharacters(u32, adjustedSize, settings.flags);

        // UVs are only valid after the atlas is packed, so make sure the
        // relevant atlas is up to date before reading them.
        ensurePacked(settings.flags);

        FT_Face face = reinterpret_cast<FT_Face>(m_face);

        // Space advance is baked at adjustedSize, so scale it to settings.size.
        float spaceWidth = 0;
        float spaceLine = settings.size * 1.2f;

        FT_Error error = FT_Load_Glyph(face, FT_Get_Char_Index(face, ' '), FT_LOAD_BITMAP_METRICS_ONLY);
        if(!error) {
            spaceWidth = (face->glyph->advance.x / 64.0f) * (static_cast<float>(settings.size) / static_cast<float>(adjustedSize));
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
                        pos.x += requestKerning(ch, previous, sdf);
                    }
                    previous = ch;

                    GlyphData *data = glyph(glyphKey(ch, adjustedSize), settings.flags);
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
        m_hasKerning = FT_HAS_KERNING( face );
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
    m_face = nullptr;

    clearAtlas(m_bitmapAtlas);
    clearAtlas(m_sdfAtlas);
}
/*!
    \internal
*/
void Font::clearAtlas(GlyphAtlas &atlas) {
    if(atlas.root->left) {
        delete atlas.root->left;
        atlas.root->left = nullptr;
    }

    if(atlas.root->right) {
        delete atlas.root->right;
        atlas.root->right = nullptr;
    }

    atlas.root->occupied = false;

    atlas.shapes.clear();

    if(atlas.page) {
        atlas.page->decRef();
        atlas.page = nullptr;
    }

    atlas.dirty = false;
}
/*!
    \internal
*/
Font::GlyphData *Font::glyph(int key, int flags) {
    PROFILE_FUNCTION();

    GlyphAtlas &atlas = atlasFor(flags);
    auto it = atlas.shapes.find(key);
    if(it != atlas.shapes.end()) {
        return &(it->second);
    }
    return nullptr;
}
/*!
    \internal
*/
Font::GlyphAtlas &Font::atlasFor(int flags) {
    return (flags & Sdf) ? m_sdfAtlas : m_bitmapAtlas;
}
/*!
    \internal
*/
const Font::GlyphAtlas &Font::atlasFor(int flags) const {
    return (flags & Sdf) ? m_sdfAtlas : m_bitmapAtlas;
}
/*!
    \internal
*/
void Font::ensurePacked(int flags) {
    GlyphAtlas &atlas = atlasFor(flags);
    if(atlas.dirty) {
        packSheets(atlas);
    }
}
/*!
    Packs all added elements of \a atlas into a glyph sheet.
    Parameter \a padding can be used to delimit elements.
*/
void Font::packSheets(GlyphAtlas &atlas) {
    PROFILE_FUNCTION();

    uint32_t atlasWidth = atlas.width;
    uint32_t atlasHeight = atlas.height;

    while(true) {
        atlas.root->w = atlasWidth;
        atlas.root->h = atlasHeight;

        uint32_t i = 0;
        for(auto &it : atlas.shapes) {
            i++;
            if(it.second.node) {
                continue;
            }
            int32_t width = (it.second.width + atlas.padding * 2);
            int32_t height = (it.second.height + atlas.padding * 2);

            AtlasNode *node = atlas.root->insert(width, height);
            if(node) {
                node->occupied = true;
                it.second.node = node;
            } else { // Not enough space. Increasing page size
                atlasWidth *= 2;
                atlasHeight *= 2;

                clearAtlas(atlas);

                break;
            }
        }

        if(i == atlas.shapes.size()) {
            break;
        }
    }

    atlas.width = atlasWidth;
    atlas.height = atlasHeight;

    Texture *page = atlas.page;
    if(page == nullptr) {
        page = Engine::objectCreate<Texture>();
        page->incRef();
        // SDF gradients must be interpolated, bitmap coverage must not be blurred.
        page->setFiltering(atlas.sdf ? Texture::Bilinear : Texture::None);
        atlas.page = page;
    }

    // A tiny half-texel offset guards against floating-point error at the
    // glyph edges. A full 0.5-texel offset is unnecessary here because padding
    // already separates glyphs, and it would clip small glyphs.
    const float halfTexelX = 0.05f / static_cast<float>(atlasWidth);
    const float halfTexelY = 0.05f / static_cast<float>(atlasHeight);

    page->resize(atlasWidth, atlasHeight);
    for(auto &it : atlas.shapes) {
        if(!it.second.copied) {
            AtlasNode *node = it.second.node;
            uint8_t *src = it.second.data.data();
            uint8_t *dst = page->surface(0).front().data();
            for(int32_t y = 0; y < it.second.height; y++) {
                uint32_t index = (node->y + y + atlas.padding) * atlasWidth + node->x + atlas.padding;
                memcpy(&dst[index], &src[y * it.second.width], it.second.width);
            }

            it.second.copied = true;

            Vector4 uvFrame;
            uvFrame.x = (node->x + atlas.padding) / static_cast<float>(atlasWidth);
            uvFrame.y = (node->y + atlas.padding) / static_cast<float>(atlasHeight);
            uvFrame.z = uvFrame.x + it.second.width / static_cast<float>(atlasWidth);
            uvFrame.w = uvFrame.y + it.second.height / static_cast<float>(atlasHeight);

            uvFrame.x += halfTexelX;
            uvFrame.y -= halfTexelY;
            uvFrame.z += halfTexelX;
            uvFrame.w -= halfTexelY;

            it.second.uvs = {Vector2(uvFrame.x, uvFrame.y),
                             Vector2(uvFrame.z, uvFrame.y),
                             Vector2(uvFrame.z, uvFrame.w),
                             Vector2(uvFrame.x, uvFrame.w)};
        }
    }

    page->setDirty();
    atlas.dirty = false;
}
/*!
    Returns glyph sheet texture.
*/
Texture *Font::page() {
    PROFILE_FUNCTION();

    ensurePacked(0);

    if(m_bitmapAtlas.page == nullptr) {
        m_bitmapAtlas.page = Engine::objectCreate<Texture>();
        m_bitmapAtlas.page->incRef();
        m_bitmapAtlas.page->setFiltering(Texture::None);
    }

    return m_bitmapAtlas.page;
}
/*!
    Returns SDF glyph sheet texture.
*/
Texture *Font::sdfPage() {
    PROFILE_FUNCTION();

    ensurePacked(Sdf);

    if(m_sdfAtlas.page == nullptr) {
        m_sdfAtlas.page = Engine::objectCreate<Texture>();
        m_sdfAtlas.page->incRef();
        m_sdfAtlas.page->setFiltering(Texture::Bilinear);
    }

    return m_sdfAtlas.page;
}
