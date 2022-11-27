#include "components/textrender.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/mesh.h"
#include "resources/font.h"
#include "resources/material.h"

#include "commandbuffer.h"
#include "utils.h"

#include <array>

#define FONT     "Font"
#define MATERIAL "Material"

#define OVERRIDE "texture0"

class TextRenderPrivate : public Resource::IObserver {
public:
    TextRenderPrivate() :
        m_color(1.0f),
        m_font(nullptr),
        m_material(nullptr),
        m_mesh(Engine::objectCreate<Mesh>()),
        m_size(16),
        m_alignment(Left),
        m_kerning(true),
        m_wrap(false) {

        m_mesh->makeDynamic();

        Material *material = Engine::loadResource<Material>(".embedded/DefaultFont.shader");
        if(material) {
            m_material = material->createInstance();
        }
    }

    ~TextRenderPrivate() {
        if(m_font) {
            m_font->unsubscribe(this);
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override {
        if(resource == m_font && state == Resource::Ready) {
            composeMesh();
        }
    }

    void composeMesh() {
        TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_boundaries);
    }

    string m_text;

    Vector4 m_color;

    Vector2 m_boundaries;

    Font *m_font;

    MaterialInstance *m_material;

    Mesh *m_mesh;

    int32_t m_size;

    int m_alignment;

    bool m_kerning;

    bool m_wrap;
};
/*!
    \class TextRender
    \brief Draws a text for the 2D and 3D graphics.
    \inmodule Engine

    The TextRender component allows you to display a text in both 2D and 3D scenes.
*/

TextRender::TextRender() :
        p_ptr(new TextRenderPrivate()) {

}

TextRender::~TextRender() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void TextRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(p_ptr->m_mesh && layer & a->layers() && !p_ptr->m_text.empty()) {
        if(layer & CommandBuffer::RAYCAST) {
            buffer.setColor(CommandBuffer::idToColor(a->uuid()));
        } else {
            buffer.setColor(p_ptr->m_color);
        }
        buffer.drawMesh(a->transform()->worldTransform(), p_ptr->m_mesh, 0, layer, p_ptr->m_material);
        buffer.setColor(Vector4(1.0f));
    }
}
/*!
    Returns the text which will be drawn.
*/
string TextRender::text() const {
    return p_ptr->m_text;
}
/*!
    Changes the \a text which will be drawn.
*/
void TextRender::setText(const string text) {
    p_ptr->m_text = text;
    p_ptr->composeMesh();
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *TextRender::font() const {
    return p_ptr->m_font;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void TextRender::setFont(Font *font) {
    if(p_ptr->m_font) {
        p_ptr->m_font->unsubscribe(p_ptr);
    }
    p_ptr->m_font = font;
    if(p_ptr->m_font) {
        p_ptr->m_font->subscribe(p_ptr);
        if(p_ptr->m_material) {
            p_ptr->m_material->setTexture(OVERRIDE, p_ptr->m_font->texture());
        }
    }
    p_ptr->composeMesh();
}
/*!
    Returns an instantiated Material assigned to TextRender.
*/
Material *TextRender::material() const {
    if(p_ptr->m_material) {
        return p_ptr->m_material->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void TextRender::setMaterial(Material *material) {
    if(!p_ptr->m_material || p_ptr->m_material->material() != material) {
        if(p_ptr->m_material) {
            delete p_ptr->m_material;
            p_ptr->m_material = nullptr;
        }

        if(material) {
            p_ptr->m_material = material->createInstance();
            if(p_ptr->m_font) {
                p_ptr->m_material->setTexture(OVERRIDE, p_ptr->m_font->texture());
            }
        }
    }
}
/*!
    Returns the size of the font.
*/
int TextRender::fontSize() const {
    return p_ptr->m_size;
}
/*!
    Changes the \a size of the font.
*/
void TextRender::setFontSize(int size) {
    p_ptr->m_size = size;
    p_ptr->composeMesh();
}
/*!
    Returns the color of the text to be drawn.
*/
Vector4 &TextRender::color() const {
    return p_ptr->m_color;
}
/*!
    Changes the \a color of the text to be drawn.
*/
void TextRender::setColor(const Vector4 color) {
    p_ptr->m_color = color;
}
/*!
    Returns true if word wrap enabled; otherwise returns false.
*/
bool TextRender::wordWrap() const {
    return p_ptr->m_wrap;
}
/*!
    Sets the word \a wrap policy. Set true to enable word wrap and false to disable.
*/
void TextRender::setWordWrap(bool wrap) {
    p_ptr->m_wrap = wrap;
    p_ptr->composeMesh();
}
/*!
    Returns the boundaries of the text area. This parameter is involved in Word Wrap calculations.
*/
Vector2 &TextRender::size() const {
    return p_ptr->m_boundaries;
}
/*!
    Changes the size of \a boundaries of the text area. This parameter is involved in Word Wrap calculations.
*/
void TextRender::setSize(const Vector2 boundaries) {
    p_ptr->m_boundaries = boundaries;
    p_ptr->composeMesh();
}
/*!
    Returns text alignment policy.
*/
int TextRender::align() const {
    return p_ptr->m_alignment;
}
/*!
    Sets text \a alignment policy.
*/
void TextRender::setAlign(int alignment) {
    p_ptr->m_alignment = alignment;
    p_ptr->composeMesh();
}
/*!
    Returns true if glyph kerning enabled; otherwise returns false.
*/
bool TextRender::kerning() const {
    return p_ptr->m_kerning;
}
/*!
    Set true to enable glyph \a kerning and false to disable.
    \note Glyph kerning functionality depends on fonts which you are using. In case of font doesn't support kerning, you will not see the difference.
*/
void TextRender::setKerning(const bool kerning) {
    p_ptr->m_kerning = kerning;
    p_ptr->composeMesh();
}
/*!
    \internal
*/
void TextRender::loadData(const VariantList &data) {
    Component::loadData(data);
    p_ptr->composeMesh();
}
/*!
    \internal
*/
void TextRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(FONT);
        if(it != data.end()) {
            setFont(Engine::loadResource<Font>((*it).second.toString()));
        }
    }
    {
        auto it = data.find(MATERIAL);
        if(it != data.end()) {
            setMaterial(Engine::loadResource<Material>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap TextRender::saveUserData() const {
    VariantMap result = Component::saveUserData();
    {
        Font *o = font();
        string ref = Engine::reference(o);
        if(!ref.empty()) {
            result[FONT] = ref;
        }
    }
    {
        Material *o = material();
        string ref = Engine::reference(o);
        if(!ref.empty()) {
            result[MATERIAL] = ref;
        }
    }
    return result;
}
/*!
    \internal
*/
bool TextRender::event(Event *ev) {
    if(ev->type() == Event::LanguageChange) {
        p_ptr->composeMesh();
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
AABBox TextRender::bound() const {
    if(p_ptr->m_mesh) {
        return p_ptr->m_mesh->lod(0)->bound() * actor()->transform()->worldTransform();
    }
    return Renderable::bound();
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

            Lod lod;

            IndexVector &indices = lod.indices();
            Vector3Vector &vertices = lod.vertices();
            Vector2Vector &uv0 = lod.uv0();

            vertices.resize(length * 4);
            indices.resize(length * 6);
            uv0.resize(length * 4);

            list<float> width;
            list<uint32_t> position;

            Vector3 pos(0.0, boundaries.y - size, 0.0f);
            uint32_t previous = 0;
            uint32_t it = 0;
            uint32_t space = 0;

            Vector3 bb[2];
            bb[1].y = -spaceLine;
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
                        bb[1].x = MAX(bb[1].x, pos.x);
                        position.push_back(it);
                        pos = Vector3(0.0f, pos.y - spaceLine, 0.0f);
                        bb[1].y = MAX(bb[1].y, pos.y);

                        space = 0;
                    } break;
                    default: {
                        if(kerning) {
                            pos.x += font->requestKerning(ch, previous);
                        }
                        uint32_t index = font->atlasIndex(ch);

                        Mesh *m = font->mesh(index);
                        if(m == nullptr) {
                            continue;
                        }
                        Lod *l = m->lod(0);

                        Vector3Vector &shape = l->vertices();
                        Vector2Vector &uv = l->uv0();

                        bb[0].x = MIN(bb[0].x, shape[0].x * size);
                        bb[0].y = MIN(bb[0].y, shape[0].y * size);

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

                                bb[1].x = MAX(bb[1].x, shift - spaceWidth);
                                bb[1].y = MAX(bb[1].y, pos.y);
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
            if(wrap) {
                bb[1].x = boundaries.x;
                bb[1].y = -boundaries.y;
            } else {
                bb[1].x = MAX(bb[1].x, pos.x);
                bb[1].y = MAX(bb[1].y, pos.y);
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

            AABBox box;
            box.setBox(bb[0], bb[1]);
            lod.setBound(box);
            lod.setTopology(Mesh::Triangles);
            lod.setFlags(Mesh::Uv0);
            mesh->setLod(0, &lod);
        }
    }
}

Vector2 TextRender::cursorPosition(Font *font, int size, const string &text, int alignment, bool kerning, bool wrap, const Vector2 &boundaries) {
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

                        Mesh *m = font->mesh(index);
                        if(m == nullptr) {
                            continue;
                        }
                        Lod *l = m->lod(0);
                        Vector3Vector &shape = l->vertices();

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

#ifdef SHARED_DEFINE
#include "viewport/handles.h"

bool TextRender::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        AABBox box = bound();
        Handles::drawBox(box.center, Quaternion(), box.extent * 2.0f);
    }
    return false;
}
#endif
