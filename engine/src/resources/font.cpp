#include "font.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "log.h"

#define HEADER  "Header"
#define DATA    "Data"

static FT_Library library = nullptr;
//FT_Done_FreeType(library);

Font::Font() :
        Atlas(),
        m_Size(12),
        m_pFace(nullptr) {

    clear();

    if(FT_Init_FreeType( &library )) {
        // FT_Init_FreeType failed
    }
}

Font::~Font() {
     clear();
}

uint32_t Font::size() const {
    return m_Size;
}

uint32_t Font::atlasIndex(uint32_t character, uint32_t size) const {
    if(size == 0) {
        size    = m_Size;
    }
    uint32_t ch = character ^ size;
    auto it = m_GlyphMap.find(ch);
    if(it != m_GlyphMap.end()) {
        return (*it).second;
    }
    return 0;
}

void Font::requestCharacters(const u32string &characters, uint32_t size) {
    if(size == 0) {
        size    = m_Size;
    }

    FT_Error error  = FT_Set_Char_Size( m_pFace, size * 64, 0, 100, 0 );
    if(!error) {
        for(auto it : characters) {
            uint32_t ch = it ^ size;
            if(m_GlyphMap.find(ch) == m_GlyphMap.end() && m_pFace) {
                error   = FT_Load_Glyph( m_pFace, FT_Get_Char_Index( m_pFace, it ), FT_LOAD_DEFAULT );
                if(!error) {
                    FT_Glyph glyph;
                    error   = FT_Get_Glyph( m_pFace->glyph, &glyph );
                    if(!error) {
                        FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
                        FT_Bitmap &bitmap   = reinterpret_cast<FT_BitmapGlyph>(glyph)->bitmap;

                        FT_BBox bbox;
                        FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_pixels, &bbox);

                        Texture *t  = Engine::objectCreate<Texture>("", this);
                        t->setWidth(bitmap.width);
                        t->setHeight(bitmap.rows);

                        Vector2Vector shape;
                        shape.resize(4);
                        shape[0] = Vector2(bbox.xMin, bbox.yMax);
                        shape[1] = Vector2(bbox.xMax, bbox.yMax);
                        shape[2] = Vector2(bbox.xMax, bbox.yMin);
                        shape[3] = Vector2(bbox.xMin, bbox.yMin);

                        t->setShape(shape);

                        uint32_t size   = bitmap.width * bitmap.rows;

                        Texture::Surface s;
                        uint8_t *buffer = new uint8_t[size];
                        memcpy(buffer, bitmap.buffer, size);
                        s.push_back(buffer);
                        t->addSurface(s);

                        m_GlyphMap[ch]  = addElement(t);
                    }
                }
            }
        }
        pack(1);
        m_pTexture->apply();
    }
}

void Font::setFontName(const string &name) {
    clear();

    m_FontName  = name;

    FT_Error error  = FT_New_Face( library, m_FontName.c_str(), 0, &m_pFace );
    if(error) {
        Log(Log::ERR) << "Can't load font" << name.c_str() << "system returned error:" << error;
    }
}

uint32_t Font::length(const u32string &characters) const {
    return characters.length();
}

uint32_t Font::spaceWidth(uint32_t size) const {
    if(size == 0) {
        size    = m_Size;
    }

    FT_Error error  = FT_Set_Char_Size( m_pFace, size * 64, 0, 100, 0 );
    if(!error) {
        error   = FT_Load_Glyph( m_pFace, FT_Get_Char_Index( m_pFace, ' ' ), FT_LOAD_DEFAULT );
        if(!error) {
            return m_pFace->glyph->advance.x / 64;
        }
    }
    return 0;
}

uint32_t Font::lineHeight(uint32_t size) const {
    if(size == 0) {
        size    = m_Size;
    }

    FT_Error error  = FT_Set_Char_Size( m_pFace, size * 64, 0, 100, 0 );
    if(!error) {
        error   = FT_Load_Glyph( m_pFace, FT_Get_Char_Index( m_pFace, '\n' ), FT_LOAD_DEFAULT );
        if(!error) {
            return m_pFace->glyph->metrics.height / (float)32;
        }
    }
    return 0;
}

void Font::loadUserData(const VariantMap &data) {
    clear();

    {
        auto it = data.find(HEADER);
        if(it != data.end()) {
            VariantList header  = (*it).second.value<VariantList>();

            auto i      = header.begin();
            //Reserved
            i++;
            m_Size      = (*i).toInt();
            i++;
            string name = (*i).toString();
            i++;
            if(!name.empty()) {
                setFontName(name);
            }
        }
    }

    {
        auto it = data.find(DATA);
        if(it != data.end()) {
            m_Data  = (*it).second.toByteArray();
            FT_Error error = FT_New_Memory_Face(library, reinterpret_cast<const uint8_t *>(&m_Data[0]), m_Data.size(), 0, &m_pFace);
            if(error) {
                Log(Log::ERR) << "Can't load font. System returned error:" << error;
            }
        }
    }
}

void Font::clear() {
    FT_Done_Face(m_pFace);
}
