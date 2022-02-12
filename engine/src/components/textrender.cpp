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
#define COLOR "uni.color0"

class TextRenderPrivate : public Resource::IObserver {
public:
    TextRenderPrivate() :
        m_Color(1.0f),
        m_pFont(nullptr),
        m_pMaterial(nullptr),
        m_pMesh(Engine::objectCreate<Mesh>()),
        m_Size(16),
        m_Alignment(Left),
        m_Kerning(true),
        m_Wrap(false) {

        m_pMesh->makeDynamic();
        m_pMesh->setFlags(Mesh::Uv0);

        Material *material = Engine::loadResource<Material>(".embedded/DefaultFont.mtl");
        if(material) {
            m_pMaterial = material->createInstance();
        }
    }

    ~TextRenderPrivate() {
        if(m_pFont) {
            m_pFont->unsubscribe(this);
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override {
        if(resource == m_pFont && state == Resource::Ready) {
            composeMesh();
        }
    }

    void composeMesh() {
        TextRender::composeMesh(m_pFont, m_pMesh, m_Size, m_Text, m_Alignment, m_Kerning, m_Wrap, m_Boundaries);
    }

    string m_Text;

    Vector4 m_Color;

    Vector2 m_Boundaries;

    Font *m_pFont;

    MaterialInstance *m_pMaterial;

    Mesh *m_pMesh;

    int32_t m_Size;

    int m_Alignment;

    bool m_Kerning;

    bool m_Wrap;
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
    if(p_ptr->m_pMesh && layer & a->layers() && !p_ptr->m_Text.empty()) {
        if(layer & CommandBuffer::RAYCAST) {
            buffer.setColor(CommandBuffer::idToColor(a->uuid()));
        }
        buffer.drawMesh(a->transform()->worldTransform(), p_ptr->m_pMesh, 0, layer, p_ptr->m_pMaterial);
        buffer.setColor(Vector4(1.0f));
    }
}
/*!
    Returns the text which will be drawn.
*/
string TextRender::text() const {
    return p_ptr->m_Text;
}
/*!
    Changes the \a text which will be drawn.
*/
void TextRender::setText(const string &text) {
    p_ptr->m_Text = text;
    p_ptr->composeMesh();
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *TextRender::font() const {
    return p_ptr->m_pFont;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void TextRender::setFont(Font *font) {
    if(p_ptr->m_pFont) {
        p_ptr->m_pFont->unsubscribe(p_ptr);
    }
    p_ptr->m_pFont = font;
    if(p_ptr->m_pFont) {
        p_ptr->m_pFont->subscribe(p_ptr);
        if(p_ptr->m_pMaterial) {
            p_ptr->m_pMaterial->setTexture(OVERRIDE, p_ptr->m_pFont->texture());
        }
    }
    p_ptr->composeMesh();
}
/*!
    Returns an instantiated Material assigned to TextRender.
*/
Material *TextRender::material() const {
    if(p_ptr->m_pMaterial) {
        return p_ptr->m_pMaterial->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void TextRender::setMaterial(Material *material) {
    if(!p_ptr->m_pMaterial || p_ptr->m_pMaterial->material() != material) {
        if(p_ptr->m_pMaterial) {
            delete p_ptr->m_pMaterial;
            p_ptr->m_pMaterial = nullptr;
        }

        if(material) {
            p_ptr->m_pMaterial = material->createInstance();
            p_ptr->m_pMaterial->setVector4(COLOR, &p_ptr->m_Color);
            if(p_ptr->m_pFont) {
                p_ptr->m_pMaterial->setTexture(OVERRIDE, p_ptr->m_pFont->texture());
            }
        }
    }
}
/*!
    Returns the size of the font.
*/
int TextRender::fontSize() const {
    return p_ptr->m_Size;
}
/*!
    Changes the \a size of the font.
*/
void TextRender::setFontSize(int size) {
    p_ptr->m_Size = size;
    p_ptr->composeMesh();
}
/*!
    Returns the color of the text to be drawn.
*/
Vector4 &TextRender::color() const {
    return p_ptr->m_Color;
}
/*!
    Changes the \a color of the text to be drawn.
*/
void TextRender::setColor(const Vector4 &color) {
    p_ptr->m_Color = color;
    if(p_ptr->m_pMaterial) {
        p_ptr->m_pMaterial->setVector4(COLOR, &p_ptr->m_Color);
    }
}
/*!
    Returns true if word wrap enabled; otherwise returns false.
*/
bool TextRender::wordWrap() const {
    return p_ptr->m_Wrap;
}
/*!
    Sets the word \a wrap policy. Set true to enable word wrap and false to disable.
*/
void TextRender::setWordWrap(bool wrap) {
    p_ptr->m_Wrap = wrap;
    p_ptr->composeMesh();
}
/*!
    Returns the boundaries of the text area. This parameter is involved in Word Wrap calculations.
*/
Vector2 &TextRender::size() const {
    return p_ptr->m_Boundaries;
}
/*!
    Changes the size of \a boundaries of the text area. This parameter is involved in Word Wrap calculations.
*/
void TextRender::setSize(const Vector2 &boundaries) {
    p_ptr->m_Boundaries = boundaries;
    p_ptr->composeMesh();
}
/*!
    Returns text alignment policy.
*/
int TextRender::align() const {
    return p_ptr->m_Alignment;
}
/*!
    Sets text \a alignment policy.
*/
void TextRender::setAlign(int alignment) {
    p_ptr->m_Alignment = alignment;
    p_ptr->composeMesh();
}
/*!
    Returns true if glyph kerning enabled; otherwise returns false.
*/
bool TextRender::kerning() const {
    return p_ptr->m_Kerning;
}
/*!
    Set true to enable glyph \a kerning and false to disable.
    \note Glyph kerning functionality depends on fonts which you are using. In case of font doesn't support kerning, you will not see the difference.
*/
void TextRender::setKerning(const bool kerning) {
    p_ptr->m_Kerning = kerning;
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
    if(p_ptr->m_pMesh) {
        return p_ptr->m_pMesh->bound() * actor()->transform()->worldTransform();
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
            mesh->setBound(box);
            mesh->setTopology(Mesh::Triangles);
            mesh->setLod(0, &lod);
        }
    }
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
