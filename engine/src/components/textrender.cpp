#include "components/textrender.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/mesh.h"
#include "resources/font.h"
#include "resources/material.h"

#include "commandbuffer.h"
#include "gizmos.h"
#include "utils.h"

#include <array>

namespace {
    const char *gFont = "Font";
    const char *gOverride = "texture0";
};

/*!
    \class TextRender
    \brief Draws a text for the 2D and 3D graphics.
    \inmodule Engine

    The TextRender component allows you to display a text in both 2D and 3D scenes.
*/

TextRender::TextRender() :
        m_color(1.0f),
        m_font(nullptr),
        m_mesh(Engine::objectCreate<Mesh>()),
        m_size(16),
        m_alignment(Left),
        m_kerning(true),
        m_wrap(false) {

    m_mesh->makeDynamic();

    Material *material = Engine::loadResource<Material>(".embedded/DefaultFont.shader");
    if(material) {
        m_materials.push_back(material->createInstance());
    }
}

TextRender::~TextRender() {
    if(m_font) {
        m_font->unsubscribe(this);
    }
}
/*!
    \internal
*/
void TextRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(m_mesh && !m_materials.empty() && layer & a->layers() && !m_text.empty()) {
        buffer.setObjectId(a->uuid());
        buffer.setMaterialId(material()->uuid());
        buffer.setColor(m_color);

        buffer.drawMesh(a->transform()->worldTransform(), m_mesh, 0, layer, m_materials.front());
    }
}
/*!
    Returns the text which will be drawn.
*/
string TextRender::text() const {
    return m_text;
}
/*!
    Changes the \a text which will be drawn.
*/
void TextRender::setText(const string text) {
    m_text = text;
    composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_boundaries);
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *TextRender::font() const {
    return m_font;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void TextRender::setFont(Font *font) {
    if(m_font) {
        m_font->unsubscribe(this);
    }

    m_font = font;
    if(m_font) {
        m_font->subscribe(&TextRender::fontUpdated, this);
        if(!m_materials.empty()) {
            m_materials.front()->setTexture(gOverride, m_font->texture());
        }
    }
    composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_boundaries);
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void TextRender::setMaterial(Material *material) {
    Renderable::setMaterial(material);

    if(m_font && !m_materials.empty()) {
        m_materials.front()->setTexture(gOverride, m_font->texture());
    }
}
/*!
    Returns the size of the font.
*/
int TextRender::fontSize() const {
    return m_size;
}
/*!
    Changes the \a size of the font.
*/
void TextRender::setFontSize(int size) {
    m_size = size;
    composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_boundaries);
}
/*!
    Returns the color of the text to be drawn.
*/
Vector4 TextRender::color() const {
    return m_color;
}
/*!
    Changes the \a color of the text to be drawn.
*/
void TextRender::setColor(const Vector4 color) {
    m_color = color;
}
/*!
    Returns true if word wrap enabled; otherwise returns false.
*/
bool TextRender::wordWrap() const {
    return m_wrap;
}
/*!
    Sets the word \a wrap policy. Set true to enable word wrap and false to disable.
*/
void TextRender::setWordWrap(bool wrap) {
    m_wrap = wrap;
    composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_boundaries);
}
/*!
    Returns the boundaries of the text area. This parameter is involved in Word Wrap calculations.
*/
Vector2 TextRender::size() const {
    return m_boundaries;
}
/*!
    Changes the size of \a boundaries of the text area. This parameter is involved in Word Wrap calculations.
*/
void TextRender::setSize(const Vector2 boundaries) {
    m_boundaries = boundaries;
    composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_boundaries);
}
/*!
    Returns text alignment policy.
*/
int TextRender::align() const {
    return m_alignment;
}
/*!
    Sets text \a alignment policy.
*/
void TextRender::setAlign(int alignment) {
    m_alignment = alignment;
    composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_boundaries);
}
/*!
    Returns true if glyph kerning enabled; otherwise returns false.
*/
bool TextRender::kerning() const {
    return m_kerning;
}
/*!
    Set true to enable glyph \a kerning and false to disable.
    \note Glyph kerning functionality depends on fonts which you are using. In case of font doesn't support kerning, you will not see the difference.
*/
void TextRender::setKerning(const bool kerning) {
    m_kerning = kerning;
    composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_boundaries);
}
/*!
    \internal
*/
void TextRender::loadData(const VariantList &data) {
    Renderable::loadData(data);
    composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_boundaries);
}
/*!
    \internal
*/
void TextRender::loadUserData(const VariantMap &data) {
    Renderable::loadUserData(data);

    auto it = data.find(gFont);
    if(it != data.end()) {
        setFont(Engine::loadResource<Font>((*it).second.toString()));
    }
}
/*!
    \internal
*/
VariantMap TextRender::saveUserData() const {
    VariantMap result(Renderable::saveUserData());

    string ref = Engine::reference(font());
    if(!ref.empty()) {
        result[gFont] = ref;
    }

    return result;
}
/*!
    \internal
*/
bool TextRender::event(Event *ev) {
    if(ev->type() == Event::LanguageChange) {
        composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_boundaries);
    }

    return true;
}
/*!
    \internal
*/
void TextRender::composeComponent() {
    setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
    setText("Text");
    setColor(Vector4(1.0f));
}
/*!
    \internal
*/
AABBox TextRender::localBound() const {
    if(m_mesh) {
        return m_mesh->bound();
    }
    return Renderable::localBound();
}
/*!
    \internal
*/
void TextRender::composeMesh(Font *font, Mesh *mesh, int size, const string &text, int alignment, bool kerning, bool wrap, const Vector2 &boundaries) {
    if(font) {
        float spaceWidth = font->spaceWidth() * size;
        float spaceLine = font->lineHeight() * size;

        string data = Engine::translate(text);
        font->requestCharacters(data);

        uint32_t length = font->length(data);
        if(length) {
            u32string u32 = Utils::utf8ToUtf32(data);

            IndexVector &indices = mesh->indices();
            Vector3Vector &vertices = mesh->vertices();
            Vector2Vector &uv0 = mesh->uv0();
            Vector4Vector &colors = mesh->colors();

            vertices.resize(length * 4);
            indices.resize(length * 6);
            uv0.resize(length * 4);
            colors.resize(length * 4);

            list<float> width;
            list<uint32_t> position;

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
                            pos.x += font->requestKerning(ch, previous);
                        }
                        uint32_t index = font->atlasIndex(ch);

                        Mesh *glyph = font->mesh(index);
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
}

Vector2 TextRender::cursorPosition(Font *font, int size, const string &text, bool kerning, const Vector2 &boundaries) {
    if(font) {
        float spaceWidth = font->spaceWidth() * size;
        float spaceLine = font->lineHeight() * size;
        float cursorMid = font->cursorWidth() * 0.5f * size;

        string data = Engine::translate(text);
        font->requestCharacters(data);

        Vector2 pos(0.0, boundaries.y - size);

        uint32_t length = font->length(data);
        if(length) {
            u32string u32 = Utils::utf8ToUtf32(data);

            uint32_t previous = 0;
            uint32_t it = 0;

            for(uint32_t i = 0; i < length; i++) {
                uint32_t ch = u32[i];
                switch(ch) {
                    case ' ': {
                        pos += Vector2(spaceWidth, 0.0f);
                    } break;
                    case '\t': {
                        pos += Vector2(spaceWidth * 4, 0.0f);
                    } break;
                    case '\r': break;
                    case '\n': {
                        pos = Vector2(0.0f, pos.y - spaceLine);
                    } break;
                    default: {
                        if(kerning) {
                            pos.x += font->requestKerning(ch, previous);
                        }
                        uint32_t index = font->atlasIndex(ch);

                        Mesh *glyph = font->mesh(index);
                        if(glyph == nullptr) {
                            continue;
                        }
                        Vector3Vector &shape = glyph->vertices();

                        pos += Vector2(shape[2].x * size, 0.0f);
                        it++;
                    } break;
                }
                previous = ch;
            }
        }
        pos.x -= cursorMid;

        return pos;
    }
    return Vector2();
}

void TextRender::drawGizmosSelected() {
    AABBox box = bound();
    Gizmos::drawWireBox(box.center, box.extent * 2.0f, Vector4(1.0f));
}

void TextRender::fontUpdated(int state, void *ptr) {
    if(state == ResourceState::Ready) {
        TextRender *p = static_cast<TextRender *>(ptr);
        composeMesh(p->m_font, p->m_mesh, p->m_size, p->m_text, p->m_alignment, p->m_kerning, p->m_wrap, p->m_boundaries);
    }
}
