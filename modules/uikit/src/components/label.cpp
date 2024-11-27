#include "components/label.h"

#include "components/recttransform.h"

#include "stylesheet.h"

#include <components/actor.h>
#include <components/textrender.h>
#include <components/transform.h>

#include <resources/resource.h>
#include <resources/material.h>
#include <resources/mesh.h>
#include <resources/font.h>

#include <commandbuffer.h>
#include <utils.h>

namespace  {
    const char *gFont = "Font";

    const char *gColor = "mainColor";
    const char *gTexture = "mainTexture";
    const char *gClipRect = "clipRect";
    const char *gWeight = "weight";
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
        m_fontWeight(0.5f),
        m_kerning(true),
        m_wrap(false),
        m_dirty(true) {

    m_mesh->makeDynamic();

    Material *material = Engine::loadResource<Material>(".embedded/DefaultFont.shader");
    if(material) {
        m_material = material->createInstance();
        m_material->setFloat(gWeight, &m_fontWeight);
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
void Label::draw(CommandBuffer &buffer) {
    if(m_material && !m_text.empty()) {
        m_material->setTransform(transform());

        if(m_dirty) {
            m_mesh->setName(actor()->name());
            TextRender::composeMesh(m_font, m_mesh, m_size, m_text, m_alignment, m_kerning, m_wrap, m_meshSize);
            m_dirty = false;
        }

        buffer.drawMesh(m_mesh, 0, CommandBuffer::UI, *m_material);
    }
}
/*!
    \internal
    Applies style settings assigned to widget.
*/
void Label::applyStyle() {
    Widget::applyStyle();

    auto it = m_styleRules.find("color");
    if(it != m_styleRules.end()) {
        setColor(StyleSheet::toColor(it->second.second));
    }

    it = m_styleRules.find("font-size");
    if(it != m_styleRules.end()) {
        setFontSize(stoul(it->second.second, nullptr));
    }

    it = m_styleRules.find("font-weight");
    if(it != m_styleRules.end()) {
        m_fontWeight = 0.5f;
        if(it->second.second == "normal") {
            m_fontWeight = 0.5f;
        } else if(it->second.second == "bold") {
            m_fontWeight = 0.8f;
        } else if(it->second.second.back() == '0') {
            m_fontWeight = stof(it->second.second) / 100.0f + 0.1f;
        }

        if(m_material) {
            m_material->setFloat(gWeight, &m_fontWeight);
        }
    }

    it = m_styleRules.find("white-space");
    if(it != m_styleRules.end()) {
        bool wordWrap = it->second.second != "nowrap";
        setWordWrap(wordWrap);
    }
}
/*!
    Returns the text which will be drawn.
*/
std::string Label::text() const {
    return m_text;
}
/*!
    Changes the \a text which will be drawn.
*/
void Label::setText(const std::string text) {
    if(m_text != text) {
        m_text = text;
        m_dirty = true;
    }
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
    if(m_font != font) {
        if(m_font) {
            m_font->unsubscribe(this);
        }

        m_font = font;
        if(m_font) {
            m_font->subscribe(&Label::fontUpdated, this);

            if(m_material) {
                m_material->setTexture(gTexture, m_font->page());
            }
        }

        m_dirty = true;
    }
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
    if(m_size != size) {
        m_size = size;
        m_dirty = true;
    }
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

    if(m_material) {
        m_material->setVector4(gColor, &m_color);
    }
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
    if(m_wrap != wrap) {
        m_wrap = wrap;
        m_dirty = true;
    }
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
    if(m_alignment != alignment) {
        m_alignment = alignment;
        m_dirty = true;
    }
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
    if(m_kerning != kerning) {
        m_kerning = kerning;
        m_dirty = true;
    }
}
/*!
    Returns a \a position for virtual cursor.
*/
Vector2 Label::cursorAt(int position) {
    std::u32string u32 = Utils::utf8ToUtf32(m_text);
    return TextRender::cursorPosition(m_font, m_size, Utils::utf32ToUtf8(u32.substr(0, position)), m_kerning, m_meshSize);
}
/*!
    \internal
*/
void Label::setClipOffset(const Vector2 &offset) {
    if(m_clipOffset != offset) {
        m_clipOffset = offset;
        if(m_material) {
            Vector4 clipRect(m_clipOffset, m_meshSize.x + m_clipOffset.x, m_meshSize.y + m_clipOffset.y);

            m_material->setVector4(gClipRect, &clipRect);
        }
    }
}
/*!
    \internal
*/
void Label::loadData(const VariantList &data) {
    Component::loadData(data);

    m_dirty = true;
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
        std::string ref = Engine::reference(o);
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
        m_dirty = true;
    }
    return true;
}
/*!
    \internal
*/
void Label::boundChanged(const Vector2 &size) {
    Widget::boundChanged(size);

    if(m_meshSize != size) {
        m_meshSize = size;
        m_dirty = true;

        if(m_material) {
            Vector4 clipRect(m_clipOffset, m_meshSize.x + m_clipOffset.x, m_meshSize.y + m_clipOffset.y);

            m_material->setVector4(gClipRect, &clipRect);
        }
    }
}
/*!
    \internal
*/
void Label::composeComponent() {
    Widget::composeComponent();

    setFontSize(14);
    setColor(Vector4(1.0f));
    setAlign(Alignment::Center | Alignment::Middle);
    setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
    setText("Text");

    RectTransform *rect = rectTransform();
    if(rect) {
        rect->setSize(Vector2(100.0f, fontSize() + 2));
    }
}
/*!
    \internal
*/
void Label::fontUpdated(int state, void *ptr) {
    if(state == Resource::Ready) {
        Label *p = static_cast<Label *>(ptr);
        p->m_dirty = true;
    }
}
