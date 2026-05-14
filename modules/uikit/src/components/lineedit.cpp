#include "components/lineedit.h"

#include "components/label.h"
#include "components/frame.h"
#include "components/recttransform.h"
#include "components/canvas.h"

#include <components/actor.h>

#include <resources/mesh.h>
#include <resources/font.h>

#include <input.h>
#include <timer.h>
#include <commandbuffer.h>
#include <pipelinecontext.h>

#include <algorithm>
#include <cstring>

#define HOLD_TIME 0.1f

namespace {
    const char *gColor("mainColor");
    const char *gTexture("mainTexture");
    const char *gUseSDF("useSdf");

    const char *gDefaultFont(".embedded/DefaultFont.shader");
    const char *gDefaultSprite(".embedded/DefaultSprite.shader");
    const char *gDefaultFrame(".embedded/Frame.shader");
}

/*!
    \class LineEdit
    \brief The LineEdit class is a UI component that allows users to input text.
    \inmodule Gui

    The LineEdit class provides a user interface for text input, supporting text editing, cursor positioning, and input handling.
    It inherits functionality from the Widget class and extends it to handle text-related features and animations.
*/

LineEdit::LineEdit() :
        m_backgroundColor(0.5f, 0.5f, 0.5f, 1.0f),
        m_textColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_font(nullptr),
        m_textMesh(Engine::objectCreate<Mesh>()),
        m_cursorPosition(0),
        m_fontSize(14),
        m_textPosition(0.0f),
        m_cursorBlinkRate(0.85f),
        m_cursorBlinkCurrent(0.0f),
        m_cursorRepeatHold(0.0f),
        m_hovered(false),
        m_focused(false),
        m_cursorVisible(false) {

    m_textMesh->makeDynamic();

    Material *fontMaterial = Engine::loadResource<Material>(gDefaultFont);
    if(fontMaterial) {
        m_fontMaterial = fontMaterial->createInstance();
    }

    Material *cursorMaterial = Engine::loadResource<Material>(gDefaultSprite);
    if(cursorMaterial) {
        m_cursorMaterial = cursorMaterial->createInstance();
        m_cursorMaterial->setTexture(gTexture, PipelineContext::whiteTexture());
    }

    Material *frameMaterial = Engine::loadResource<Material>(gDefaultFrame);
    if(frameMaterial) {
        m_frameMaterial = frameMaterial->createInstance();
    }

    m_cursorTransform.scale(Vector3(2.0f, 16.0f, 1.0f));
}
/*!
    \internal
*/
void LineEdit::draw() {
    Canvas *canvas = LineEdit::canvas();

    RectTransform *rect = rectTransform();
    if(m_textDirty && m_font) {
        m_font->composeMesh(m_textMesh, m_text, m_fontSize,
                            Alignment::Left | Alignment::Middle, 0, rect->size());

        m_textMesh->setColors(Vector4Vector(m_textMesh->vertices().size(), Vector4(1.0f)));

        m_fontMaterial->setTexture(gTexture, m_font->page());

        m_textDirty = false;
    }

    // Draw background
    if(m_backgroundImage) {
    } else { // drawing a rect
        canvas->drawRect(m_frameMaterial, rect);
    }

    // Draw text
    canvas->setClipRegion(rect->clipRegion());
    m_fontMaterial->setTransform(rect->worldTransform(), 0, rect->hash());
    canvas->drawMesh(m_textMesh, m_fontMaterial);

    // Draw cursor
    uint32_t hash;
    std::memcpy(&hash, &m_cursorTransform[12], sizeof(float));

    if(m_cursorVisible && m_cursorMaterial) {
        m_cursorMaterial->setTransform(rect->worldTransform() * m_cursorTransform, 0, hash);
        canvas->drawRect(m_cursorMaterial, nullptr);
    }
    canvas->disableClip();

    Widget::draw();
}
/*!
    Returns the current text entered into the TextInput.
*/
TString LineEdit::text() const {
    return m_text;
}
/*!
    Sets the \a text in the TextInput.
*/
void LineEdit::setText(const TString &text) {
    m_text = text;
    m_textDirty = true;

    if(m_cursorPosition == 0) {
        m_cursorPosition = m_text.toUtf32().size();
        recalcCursor();
    }
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *LineEdit::font() const {
    return m_font;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void LineEdit::setFont(Font *font) {
    if(m_font != font) {
        if(m_font) {
            m_font->unsubscribe(this);
        }

        m_font = font;
        if(m_font) {
            m_font->subscribe(&LineEdit::fontUpdated, this);
        }

        m_textDirty = true;
    }
}
/*!
    Returns color of the text.
*/
Vector4 LineEdit::textColor() const {
    return m_textColor;
}
/*!
    Sets the \a color of the text.
*/
void LineEdit::setTextColor(const Vector4 &color) {
    m_textColor = color;

    if(m_fontMaterial) {
        m_fontMaterial->setVector4(gColor, &m_textColor);
    }
}
/*!
    Returns the background color.
*/
Vector4 LineEdit::backgroundColor() const {
    return m_backgroundColor;
}
/*!
    Sets background \a color.
*/
void LineEdit::setBackgroundColor(const Vector4 &color) {
    m_backgroundColor = color;
}

void LineEdit::focusIn() {
    emitSignal(_SIGNAL(focusIn()));
}

void LineEdit::focusOut() {
    emitSignal(_SIGNAL(focusOut()));
}

void LineEdit::editingFinished() {
    emitSignal(_SIGNAL(editingFinished()));
}
/*!
    \internal
    Overrides the update method to handle text input and cursor animation.
*/
void LineEdit::update(const Vector2 &pos) {
    if(Widget::focusWidget() == this) {
        bool breakCursorFlashing = false;

        std::u32string u32 = m_text.toUtf32();
        if(Input::isKey(Input::KEY_BACKSPACE) || Input::isKey(Input::KEY_DELETE)) {
            breakCursorFlashing = true;
            if(m_cursorRepeatHold < 0.0f) {
                if(Input::isKey(Input::KEY_BACKSPACE) && m_cursorPosition >= 0) {
                    m_cursorPosition--;
                }
                if(m_cursorPosition >= 0) {
                    u32.erase(m_cursorPosition, 1);
                    setText(TString::fromUtf32(u32));
                } else {
                    m_cursorPosition = 0;
                }
                recalcCursor();

                m_cursorRepeatHold = HOLD_TIME;
            } else {
                m_cursorRepeatHold -= Timer::deltaTime();
            }
        } else if(Input::isKeyUp(Input::KEY_BACKSPACE) || Input::isKeyUp(Input::KEY_DELETE)) {
            m_cursorRepeatHold = 0.0f;
        }

        if(Input::isKey(Input::KEY_LEFT) && m_cursorPosition > 0) {
            breakCursorFlashing = true;
            if(m_cursorRepeatHold < 0.0f) {
                m_cursorPosition--;
                recalcCursor();

                m_cursorRepeatHold = HOLD_TIME;
            } else {
                m_cursorRepeatHold -= Timer::deltaTime();
            }
        } else if(Input::isKeyUp(Input::KEY_LEFT)) {
            m_cursorRepeatHold = 0.0f;
        }

        if(Input::isKey(Input::KEY_RIGHT) && m_cursorPosition < u32.size()) {
            breakCursorFlashing = true;
            if(m_cursorRepeatHold < 0.0f) {
                m_cursorPosition++;
                recalcCursor();

                m_cursorRepeatHold = HOLD_TIME;
            } else {
                m_cursorRepeatHold -= Timer::deltaTime();
            }
        } else if(Input::isKeyUp(Input::KEY_RIGHT)){
            m_cursorRepeatHold = 0.0f;
        }

        if(Input::isKeyDown(Input::KEY_ENTER) || Input::isKeyDown(Input::KEY_KP_ENTER)) {
            editingFinished();
        } else {
            TString sub = Input::inputString();
            std::string s = sub.toStdString();
            s.erase(remove_if(s.begin(), s.end(), [](unsigned char c) { return (c < 32);}), s.end());
            sub = s;
            if(!sub.isEmpty()) {
                breakCursorFlashing = true;
                std::u32string u32sub = sub.toUtf32();
                u32.insert(m_cursorPosition, u32sub);
                m_cursorPosition += u32sub.size();
                setText(TString::fromUtf32(u32));// for password •●●
                recalcCursor();
            }
        }

        if(breakCursorFlashing) {
            m_cursorVisible = true;
        } else {
            m_cursorBlinkCurrent += Timer::deltaTime();
            if(m_cursorBlinkCurrent >= m_cursorBlinkRate) {
                m_cursorBlinkCurrent = 0.0f;
                m_cursorVisible = !m_cursorVisible;
            }
        }
    } else {
        if(m_focused) {
            focusOut();
            m_focused = false;
            m_cursorVisible = false;
        }
    }

    Widget::update(pos);

    bool hover = isHovered(pos);
    if(m_hovered != hover) {
        m_hovered = hover;
    }

    if(m_hovered) {
        if(!m_focused && (Input::isMouseButtonDown(0) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN))) {
            Widget::setFocusWidget(this);

            focusIn();
            m_focused = true;
        }
    }
}
/*!
    \internal
    Overrides the composeComponent method to create the text input component.
*/
void LineEdit::composeComponent() {
    Widget::composeComponent();

    setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
    setText("");

    RectTransform *rect = rectTransform();
    if(rect) {
        rect->setSize(Vector2(100.0f, 30.0f));
        rect->setPadding(Vector4(2.0f));
    }

    recalcCursor();
}
/*!
    \internal
    Recalculates the cursor position based on the current text and adjusts the text position accordingly.
*/
void LineEdit::recalcCursor() {
    float pos = (m_cursorPosition > 0) ? cursorAt(m_cursorPosition) : 0.0f;

    m_cursorTransform[12] = pos + rectTransform()->padding().w;
    m_cursorTransform[13] = rectTransform()->size().y * 0.5f;

    float x = m_textPosition;
    float size = m_font->textWidth(m_text, m_fontSize, 0);
    float gap = pos + x;
    if(gap > size) {
        m_textPosition = size - pos;
    } else if(gap < 0.0f) {
        m_textPosition = x - gap;
    }
    m_cursorTransform[12] += m_textPosition;
}
/*!
    Returns a \a position for virtual cursor.
*/
float LineEdit::cursorAt(int position) const {
    std::u32string u32 = m_text.toUtf32();
    return m_font->textWidth(TString::fromUtf32(u32.substr(0, position)), m_fontSize, 0);
}
/*!
    \internal
*/
void LineEdit::fontUpdated(int state, void *ptr) {
    if(state == Resource::Ready) {
        LineEdit *p = static_cast<LineEdit *>(ptr);
        p->m_textDirty = true;
    }
}
