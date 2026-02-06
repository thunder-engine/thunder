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

namespace  {
    const char *gFont("Font");

    const char *gColor("mainColor");
    const char *gTexture("mainTexture");
    const char *gWeight("weight");
    const char *gUseSDF("useSdf");
};

/*!
    \class Label
    \brief Draws a text for the UI.
    \inmodule Gui

    The Label class is a graphical user interface (GUI) element that is used to display text within the application window.
    It is a fundamental component in UI design, providing a way to present information, instructions, or labels for other interactive elements like buttons or text fields.
*/

Label::Label() :
        m_color(1.0f),
        m_font(nullptr),
        m_material(nullptr),
        m_mesh(Engine::objectCreate<Mesh>()),
        m_size(16),
        m_alignment(Left),
        m_fontWeight(0.5f),
        m_flags(Font::Wrap),
        m_dirty(true),
        m_translated(false) {

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
    m_font = nullptr;
}
/*!
    \internal
*/
void Label::draw(CommandBuffer &buffer) {
    if(m_material && !m_text.isEmpty()) {
        m_material->setTransform(transform());

        if(m_dirty && m_font) {
            m_mesh->setName(actor()->name());
            m_font->composeMesh(m_mesh,
                                m_translated ? Engine::translate(m_text) : m_text,
                                m_size, m_alignment, m_flags, m_meshSize);

            m_mesh->setColors(Vector4Vector(m_mesh->vertices().size(), Vector4(1.0f)));

            m_material->setTexture(gTexture, m_font->page());
            bool sdf = m_flags & Font::Sdf;
            m_material->setBool(gUseSDF, &sdf);

            m_dirty = false;
        }

        buffer.drawMesh(m_mesh, 0, Material::Translucent, *m_material);
    }

    Widget::draw(buffer);
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
        setFontSize(it->second.second.toInt());
    }

    it = m_styleRules.find("font-weight");
    if(it != m_styleRules.end()) {
        m_fontWeight = 0.5f;
        if(it->second.second == "normal") {
            m_fontWeight = 0.5f;
        } else if(it->second.second == "bold") {
            m_fontWeight = 0.8f;
        } else if(it->second.second.back() == '0') {
            m_fontWeight = it->second.second.toFloat() / 100.0f + 0.1f;
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
TString Label::text() const {
    return m_text;
}
/*!
    Changes the \a text which will be drawn.
*/
void Label::setText(const TString &text) {
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
void Label::setColor(const Vector4 &color) {
    m_color = color;

    if(m_material) {
        m_material->setVector4(gColor, &m_color);
    }
}
/*!
    Returns true if text in label must be translated; othewise returns false.
*/
bool Label::translated() const {
    return m_translated;
}
/*!
    Sets \a enable or disable translation from dictionary for current label.
*/
void Label::setTranslated(bool enable) {
    if(m_translated != enable) {
        m_translated = enable;
        m_dirty = true;
    }
}
/*!
    Returns true if word wrap enabled; otherwise returns false.
*/
bool Label::wordWrap() const {
    return m_flags & Font::Wrap;
}
/*!
    Sets the word \a wrap policy. Set true to enable word wrap and false to disable.
*/
void Label::setWordWrap(bool wrap) {
    if(wordWrap() != wrap) {
        if(wrap) {
            m_flags |= Font::Wrap;
        } else {
            m_flags &= ~Font::Wrap;
        }
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
    return m_flags & Font::Kerning;
}
/*!
    Set true to \a enable glyph kerning and false to disable.
    \note Glyph kerning functionality depends on fonts which you are using. In case of font doesn't support kerning, you will not see the difference.
*/
void Label::setKerning(const bool enable) {
    if(kerning() != enable) {
        if(enable) {
            m_flags |= Font::Kerning;
        } else {
            m_flags &= ~Font::Kerning;
        }
        m_dirty = true;
    }
}
/*!
    Returns a \a position for virtual cursor.
*/
Vector2 Label::cursorAt(int position) {
    std::u32string u32 = m_text.toUtf32();
    return Vector2(m_font->textWidth(TString::fromUtf32(u32.substr(0, position)), m_size, m_flags), 0.0f);
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
        TString ref = Engine::reference(o);
        if(!ref.isEmpty()) {
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
