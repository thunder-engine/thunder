#include "components/gui/label.h"

#include "components/gui/recttransform.h"

#include "components/actor.h"
#include "components/textrender.h"
#include "components/transform.h"

#include "resources/resource.h"
#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/font.h"

#include <commandbuffer.h>
#include <utils.h>

namespace  {
    const char *gFont = "Font";
    const char *gOverride = "texture0";

    const char *gClipRect = "uni.clipRect";
};

class LabelPrivate : public Resource::IObserver {
public:
    explicit LabelPrivate(Label *label) :
        m_color(1.0f),
        m_label(label),
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

    ~LabelPrivate() {
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
        TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);
    }

    string m_text;

    Vector4 m_color;

    Vector2 m_meshSize;

    Vector2 m_clipOffset;

    Label *m_label;

    Font *m_font;

    MaterialInstance *m_material;

    Mesh *m_mesh;

    int32_t m_size;

    int m_alignment;

    bool m_kerning;

    bool m_wrap;
};

/*!
    \class Label
    \brief Draws a text for the UI.
    \inmodule Gui

    The Label component allows you to display a text in UI.
*/

Label::Label() :
        p_ptr(new LabelPrivate(this)) {

}

Label::~Label() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void Label::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(p_ptr->m_mesh && !p_ptr->m_text.empty()) {
        if(layer & CommandBuffer::RAYCAST) {
            buffer.setColor(CommandBuffer::idToColor(a->uuid()));
        } else {
            buffer.setColor(p_ptr->m_color);
        }
        buffer.drawMesh(rectTransform()->worldTransform(), p_ptr->m_mesh, 0, layer, p_ptr->m_material);
        buffer.setColor(Vector4(1.0f));
    }
}
/*!
    Returns the text which will be drawn.
*/
string Label::text() const {
    return p_ptr->m_text;
}
/*!
    Changes the \a text which will be drawn.
*/
void Label::setText(const string text) {
    p_ptr->m_text = text;
    p_ptr->composeMesh();
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *Label::font() const {
    return p_ptr->m_font;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void Label::setFont(Font *font) {
    if(p_ptr->m_font) {
        p_ptr->m_font->unsubscribe(p_ptr);
    }
    p_ptr->m_font = font;
    if(p_ptr->m_font) {
        p_ptr->m_font->subscribe(p_ptr);
        if(p_ptr->m_material) {
            p_ptr->m_material->setTexture(gOverride, p_ptr->m_font->texture());
        }
    }
    p_ptr->composeMesh();
}
/*!
    Returns the size of the font.
*/
int Label::fontSize() const {
    return p_ptr->m_size;
}
/*!
    Changes the \a size of the font.
*/
void Label::setFontSize(int size) {
    p_ptr->m_size = size;
    p_ptr->composeMesh();
}
/*!
    Returns the color of the text to be drawn.
*/
Vector4 Label::color() const {
    return p_ptr->m_color;
}
/*!
    Changes the \a color of the text to be drawn.
*/
void Label::setColor(const Vector4 color) {
    p_ptr->m_color = color;
}
/*!
    Returns true if word wrap enabled; otherwise returns false.
*/
bool Label::wordWrap() const {
    return p_ptr->m_wrap;
}
/*!
    Sets the word \a wrap policy. Set true to enable word wrap and false to disable.
*/
void Label::setWordWrap(bool wrap) {
    p_ptr->m_wrap = wrap;
    p_ptr->composeMesh();
}
/*!
    Returns text alignment policy.
*/
int Label::align() const {
    return p_ptr->m_alignment;
}
/*!
    Sets text \a alignment policy.
*/
void Label::setAlign(int alignment) {
    p_ptr->m_alignment = alignment;
    p_ptr->composeMesh();
}
/*!
    Returns true if glyph kerning enabled; otherwise returns false.
*/
bool Label::kerning() const {
    return p_ptr->m_kerning;
}
/*!
    Set true to enable glyph \a kerning and false to disable.
    \note Glyph kerning functionality depends on fonts which you are using. In case of font doesn't support kerning, you will not see the difference.
*/
void Label::setKerning(const bool kerning) {
    p_ptr->m_kerning = kerning;
    p_ptr->composeMesh();
}
/*!
    Returns a \a position for virtual cursor.
*/
Vector2 Label::cursorAt(int position) {
    u32string u32 = Utils::utf8ToUtf32(p_ptr->m_text);
    return TextRender::cursorPosition(p_ptr->m_font, p_ptr->m_size, Utils::utf32ToUtf8(u32.substr(0, position)), p_ptr->m_kerning, p_ptr->m_meshSize);
}
/*!
    \internal
*/
void Label::setClipOffset(const Vector2 &offset) {
    p_ptr->m_clipOffset = offset;
    if(p_ptr->m_material) {
        Vector4 clipRect(p_ptr->m_clipOffset, p_ptr->m_meshSize.x + p_ptr->m_clipOffset.x, p_ptr->m_meshSize.y + p_ptr->m_clipOffset.y);

        p_ptr->m_material->setVector4(gClipRect, &clipRect);
    }
}
/*!
    \internal
*/
void Label::loadData(const VariantList &data) {
    Component::loadData(data);
    p_ptr->composeMesh();
}
/*!
    \internal
*/
void Label::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(gFont);
        if(it != data.end()) {
            setFont(Engine::loadResource<Font>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap Label::saveUserData() const {
    VariantMap result = Widget::saveUserData();
    {
        Font *o = font();
        string ref = Engine::reference(o);
        if(!ref.empty()) {
            result[gFont] = ref;
        }
    }
    return result;
}
/*!
    \internal
*/
bool Label::event(Event *ev) {
    if(ev->type() == Event::LanguageChange) {
        p_ptr->composeMesh();
    }
    return true;
}
/*!
    \internal
*/
void Label::boundChanged(const Vector2 &size) {
    Widget::boundChanged(size);
    p_ptr->m_meshSize = size;
    p_ptr->composeMesh();

    if(p_ptr->m_material) {
        Vector4 clipRect(p_ptr->m_clipOffset, p_ptr->m_meshSize.x + p_ptr->m_clipOffset.x, p_ptr->m_meshSize.y + p_ptr->m_clipOffset.y);

        p_ptr->m_material->setVector4(gClipRect, &clipRect);
    }
}
/*!
    \internal
*/
void Label::composeComponent() {
    Widget::composeComponent();

    setFontSize(14);
    setColor(Vector4(1.0f));
    setAlign(Alignment::Middle);
    setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
    setText("Text");

    RectTransform *t = rectTransform();
    t->setAnchors(Vector2(0, 0), Vector2(1, 1));
}
