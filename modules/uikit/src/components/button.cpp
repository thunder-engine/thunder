#include "components/button.h"

#include <resources/material.h>
#include <resources/font.h>

#include "components/canvas.h"
#include "components/recttransform.h"

#include "stylesheet.h"

namespace {
    const char *gOverride("mainTexture");
    const char *gColor("mainColor");

    const char *gUseSDF("useSdf");

    const char *gDefaultSprite(".embedded/DefaultUI.shader");
    const char *gDefaultFont(".embedded/DefaultFont.shader");

    const char *gCssColor("color");
    const char *gCssIconSize("icon-size");
    const char *gCssFontSize("font-size");
}

/*!
    \class Button
    \brief The Button class represents a push button class.
    \inmodule Gui

    The Button class represents a push button element in a graphical user interface (GUI).
    It is a fundamental UI component that allows users to trigger actions or commands through a simple click or press.
*/

Button::Button() :
        m_textColor(Vector4(1.0f)),
        m_iconSize(16.0f),
        m_icon(nullptr),
        m_font(nullptr),
        m_iconMesh(Engine::objectCreate<Mesh>()),
        m_textMesh(Engine::objectCreate<Mesh>()),
        m_fontMaterial(nullptr),
        m_iconMaterial(nullptr),
        m_fontSize(14),
        m_rotation(0.0f),
        m_translated(false),
        m_dirtyIcon(true),
        m_dirtyText(true) {

    m_textMesh->makeDynamic();

    Material *fontMaterial = Engine::loadResource<Material>(gDefaultFont);
    if(fontMaterial) {
        m_fontMaterial = fontMaterial->createInstance();
        int sdf = 0;
        m_fontMaterial->setInteger(gUseSDF, &sdf);
    }

    Material *spriteMaterial = Engine::loadResource<Material>(gDefaultSprite);
    if(spriteMaterial) {
        m_iconMaterial = spriteMaterial->createInstance();

        Vector4 color(1.0f);
        m_iconMaterial->setVector4(gColor, &color);
    }
}

Button::~Button() {
    delete m_textMesh;

    delete m_iconMaterial;
    delete m_fontMaterial;
}
/*!
    Returns the text displayed on the button.
*/
TString Button::text() const {
    return m_text;
}
/*!
    Sets the \a text displayed on the button.
*/
void Button::setText(const TString &text) {
    m_text = text;
    m_dirtyText = true;
    repaint();
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *Button::font() const {
    return m_font;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void Button::setFont(Font *font) {
    if(m_font != font) {
        if(m_font) {
            m_font->unsubscribe(this);
        }

        m_font = font;
        if(m_font) {
            m_font->subscribe(&Button::fontUpdated, this);
        }

        m_dirtyText = true;
        repaint();
    }
}
/*!
    Returns the size of the font.
*/
int Button::fontSize() const {
    return m_fontSize;
}
/*!
    Changes the \a size of the font.
*/
void Button::setFontSize(int size) {
    if(m_fontSize != size) {
        m_fontSize = size;
        m_dirtyText = true;
        repaint();

#ifdef SHARED_DEFINE
        if(!isSubWidget() && !isSignalsBlocked()) {
            StyleSheet::setStyleProperty(this, gCssFontSize, TString::number(m_fontSize) + "px");
        }
#endif
    }
}
/*!
    Returns the normal color of the button.
*/
Vector4 Button::textColor() const {
    return m_textColor;
}
/*!
    Sets the normal \a color of the button.
*/
void Button::setTextColor(const Vector4 &color) {
    m_textColor = color;

    if(m_fontMaterial) {
        m_fontMaterial->setVector4(gColor, &m_textColor);
    }
    repaint();

#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        StyleSheet::setStyleProperty(this, gCssColor, StyleSheet::toColor(m_textColor));
    }
#endif
}
/*!
    Returns the icon shown on the button.
*/
Sprite *Button::icon() const {
    return m_icon;
}
/*!
    Sets the \a icon shown on the button.
*/
void Button::setIcon(Sprite *icon) {
    if(m_icon != icon) {
        m_icon = icon;
        m_dirtyIcon = true;
        repaint();
    }
}
/*!
    Returns the size of the icon.
*/
Vector2 Button::iconSize() const {
    return m_iconSize;
}
/*!
    Sets the \a size of the icon.
*/
void Button::setIconSize(const Vector2 &size) {
    m_iconSize = size;
    m_dirtyIcon = true;
    repaint();

#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        updateStyleProperty(gCssIconSize, m_iconSize.v, 2);
    }
#endif
}
/*!
    Sets icon rotation \a angle.
*/
void Button::setIconRotation(float angle) {
    m_rotation = angle;
}

float Button::contentWidth() const {
    float result = 0.0f;
    if(m_font) {
        result += m_font->textWidth(m_text, m_fontSize, 0);
    }

    if(m_icon) {
        result += m_iconSize.x;
    }

    return result;
}
/*!
    \internal
*/
void Button::draw() {
    AbstractButton::draw();

    RectTransform *rect = rectTransform();
    Vector2 size(rect->size());

    Matrix4 mat(rect->worldTransform());

    Vector2 scl(rect->worldScale());
    mat[12] += size.x * 0.5f * scl.x;
    mat[13] += size.y * 0.5f * scl.y;

    uint32_t hash = rect->hash();
    Mathf::hashCombine(hash, mat[12]);
    Mathf::hashCombine(hash, mat[13]);

    float offset = 0.0f;
    Canvas *canvas = Button::canvas();
    if(m_fontMaterial && !m_text.isEmpty()) {
        if(m_dirtyText && m_font) {
            m_textMesh->setName(actor()->name());
            const Font::Settings settings = {m_fontSize, Alignment::Center | Alignment::Middle, 0, m_textColor};
            m_font->composeMesh(m_textMesh, m_translated ? Engine::translate(m_text) : m_text, settings);

            m_fontMaterial->setTexture(gOverride, m_font->page());

            m_dirtyText = false;
        }

        auto &vertices = m_textMesh->vertices();
        if(!vertices.empty()) {
            offset = vertices.front().x - m_iconSize.x;
        }
        m_fontMaterial->setTransform(mat, 0, hash);
        canvas->drawMesh(m_textMesh, m_fontMaterial);
    }

    if(m_icon) {
        if(m_dirtyIcon) {
            m_iconMaterial->setTexture(gOverride, m_icon->texture());
            m_icon->composeMesh(m_iconMesh, Sprite::Sliced, m_iconSize);

            m_dirtyIcon = false;
        }

        Matrix3 rot;
        rot.rotate(Vector3(0.0f, 0.0f, 1.0f), m_rotation);

        mat[12] += offset * scl.x;
        Mathf::hashCombine(hash, offset);
        Mathf::hashCombine(hash, m_rotation);

        m_iconMaterial->setTransform(mat * rot, 0, hash);
        canvas->drawMesh(m_iconMesh, m_iconMaterial);
    }
}
/*!
    \internal
    Applies style settings assigned to widget.
*/
void Button::applyStyle() {
    AbstractButton::applyStyle();

    bool pixels;
    setIconSize(styleBlock2Length(gCssIconSize, m_iconSize, pixels));

    blockSignals(true);
    auto it = m_styleRules.find(gCssColor);
    if(it != m_styleRules.end()) {
        setTextColor(StyleSheet::toColor(it->second.second));
    }
    blockSignals(false);
}
/*!
    \internal
    Internal method called to compose the button component by adding background, label, and icon components.
*/
void Button::composeComponent() {
    AbstractButton::composeComponent();

    RectTransform *rect = rectTransform();
    rect->blockSignals(true);
    rect->setSize(Vector2(100.0f, 30.0f));
    rect->blockSignals(false);
}
/*!
    \internal
*/
void Button::fontUpdated(int state, void *ptr) {
    if(state == Resource::Ready) {
        Button *p = static_cast<Button *>(ptr);
        p->m_dirtyText = true;
        p->repaint();
    }
}
