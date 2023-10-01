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

/*!
    \class Label
    \brief Draws a text for the UI.
    \inmodule Gui

    The Label component allows you to display a text in UI.
*/

Label::Label() :
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

Label::~Label() {
    if(m_font) {
        m_font->unsubscribe(this);
    }
}
/*!
    \internal
*/
void Label::draw(CommandBuffer &buffer, uint32_t layer) {
    if(m_mesh && !m_text.empty()) {
        buffer.setObjectId(actor()->uuid());
        buffer.setMaterialId(m_material->material()->uuid());
        buffer.setColor(m_color);

        buffer.drawMesh(rectTransform()->worldTransform(), m_mesh, 0, layer, m_material);
    }
}
/*!
    Returns the text which will be drawn.
*/
string Label::text() const {
    return m_text;
}
/*!
    Changes the \a text which will be drawn.
*/
void Label::setText(const string text) {
    m_text = text;
    TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *Label::font() const {
    return m_font;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void Label::setFont(Font *font) {
    if(m_font) {
        m_font->unsubscribe(this);
    }
    m_font = font;

    if(m_font) {
        m_font->subscribe(&Label::fontUpdated, this);

        if(m_material) {
            m_material->setTexture(gOverride, m_font->texture());
        }
    }

    TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);
}
/*!
    Returns the size of the font.
*/
int Label::fontSize() const {
    return m_size;
}
/*!
    Changes the \a size of the font.
*/
void Label::setFontSize(int size) {
    m_size = size;
    TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);
}
/*!
    Returns the color of the text to be drawn.
*/
Vector4 Label::color() const {
    return m_color;
}
/*!
    Changes the \a color of the text to be drawn.
*/
void Label::setColor(const Vector4 color) {
    m_color = color;
}
/*!
    Returns true if word wrap enabled; otherwise returns false.
*/
bool Label::wordWrap() const {
    return m_wrap;
}
/*!
    Sets the word \a wrap policy. Set true to enable word wrap and false to disable.
*/
void Label::setWordWrap(bool wrap) {
    m_wrap = wrap;
    TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);
}
/*!
    Returns text alignment policy.
*/
int Label::align() const {
    return m_alignment;
}
/*!
    Sets text \a alignment policy.
*/
void Label::setAlign(int alignment) {
    m_alignment = alignment;
    TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);
}
/*!
    Returns true if glyph kerning enabled; otherwise returns false.
*/
bool Label::kerning() const {
    return m_kerning;
}
/*!
    Set true to enable glyph \a kerning and false to disable.
    \note Glyph kerning functionality depends on fonts which you are using. In case of font doesn't support kerning, you will not see the difference.
*/
void Label::setKerning(const bool kerning) {
    m_kerning = kerning;
    TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);
}
/*!
    Returns a \a position for virtual cursor.
*/
Vector2 Label::cursorAt(int position) {
    u32string u32 = Utils::utf8ToUtf32(m_text);
    return TextRender::cursorPosition(m_font, m_size, Utils::utf32ToUtf8(u32.substr(0, position)), m_kerning, m_meshSize);
}
/*!
    \internal
*/
void Label::setClipOffset(const Vector2 &offset) {
    m_clipOffset = offset;
    if(m_material) {
        Vector4 clipRect(m_clipOffset, m_meshSize.x + m_clipOffset.x, m_meshSize.y + m_clipOffset.y);

        m_material->setVector4(gClipRect, &clipRect);
    }
}
/*!
    \internal
*/
void Label::loadData(const VariantList &data) {
    Component::loadData(data);

    TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);
}
/*!
    \internal
*/
void Label::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);

    auto it = data.find(gFont);
    if(it != data.end()) {
        setFont(Engine::loadResource<Font>((*it).second.toString()));
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
        TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);
    }
    return true;
}
/*!
    \internal
*/
void Label::boundChanged(const Vector2 &size) {
    Widget::boundChanged(size);
    m_meshSize = size;
    TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);

    if(m_material) {
        Vector4 clipRect(m_clipOffset, m_meshSize.x + m_clipOffset.x, m_meshSize.y + m_clipOffset.y);

        m_material->setVector4(gClipRect, &clipRect);
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

void Label::fontUpdated(int state, void *ptr) {
    if(state == ResourceState::Ready) {
        Label *p = static_cast<Label *>(ptr);
        TextRender::composeMesh(p->m_font, p->m_mesh, p->m_size, p->m_text, p->m_alignment, p->m_kerning, p->m_wrap, p->m_meshSize);
    }
}
