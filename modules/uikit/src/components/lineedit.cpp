#include "components/lineedit.h"

#include "components/label.h"
#include "components/frame.h"
#include "components/recttransform.h"

#include <components/actor.h>

#include <resources/mesh.h>
#include <resources/font.h>

#include <input.h>
#include <timer.h>
#include <commandbuffer.h>
#include <pipelinecontext.h>

#include <algorithm>

#define HOLD_TIME 0.1f

namespace {
    const char *gText("text");

    const char *gLabelClass("Label");
    const char *gFrameClass("Frame");
}

/*!
    \class TextInput
    \brief The TextInput class is a UI component that allows users to input text.
    \inmodule Gui

    The TextInput class provides a user interface for text input, supporting text editing, cursor positioning, and input handling.
    It inherits functionality from the Widget class and extends it to handle text-related features and animations.
*/

LineEdit::LineEdit() :
        m_normalColor(0.5f, 0.5f, 0.5f, 1.0f),
        m_highlightedColor(0.6f, 0.6f, 0.6f, 1.0f),
        m_pressedColor(0.7f, 0.7f, 0.7f, 1.0f),
        m_textColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_cursor(PipelineContext::defaultPlane()),
        m_cursorPosition(0),
        m_fadeDuration(0.2f),
        m_currentFade(1.0f),
        m_cursorBlinkRate(0.85f),
        m_cursorBlinkCurrent(0.0f),
        m_cursorRepeatHold(0.0f),
        m_hovered(false),
        m_focused(false),
        m_cursorVisible(false) {

    Material *material = Engine::loadResource<Material>(".embedded/DefaultSprite.shader");
    if(material) {
        m_cursorMaterial = material->createInstance();
        m_cursorMaterial->setTexture("mainTexture", PipelineContext::whiteTexture());
    }

    m_cursorTransform.scale(Vector3(2.0f, 16.0f, 1.0f));
}
/*!
    \internal
*/
void LineEdit::drawSub(CommandBuffer &buffer) {
    RectTransform *rect = rectTransform();
    Vector4 scissor(rect->scissorArea());
    buffer.enableScissor(scissor.x, scissor.y, scissor.z, scissor.w);

    Widget::drawSub(buffer);

    if(m_cursorVisible && m_cursorMaterial) {
        m_cursorMaterial->setTransform(rect->worldTransform() * m_cursorTransform);
        buffer.drawMesh(m_cursor, 0, Material::Translucent, *m_cursorMaterial);
    }

    buffer.disableScissor();
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
    Label *label = textComponent();
    if(label) {
        m_text = text;
        label->setText(m_text);
        if(m_cursorPosition == 0) {
            m_cursorPosition = m_text.toUtf32().size();
            recalcCursor();
        }
    }
}
/*!
    Returns the color of the text.
*/
Vector4 LineEdit::textColor() const {
    return m_textColor;
}
/*!
    Sets the \a color of the text.
*/
void LineEdit::setTextColor(const Vector4 &color) {
    m_textColor = color;

    Label *label = textComponent();
    if(label) {
        label->setColor(m_textColor);
    }
}
/*!
    Returns the text label component.
*/
Label *LineEdit::textComponent() const {
    return static_cast<Label *>(subWidget(gText));
}
/*!
    Sets the text \a label component.
*/
void LineEdit::setTextComponent(Label *label) {
    setSubWidget(label);
}
/*!
    \internal
    Overrides the update method to handle text input and cursor animation.
*/
void LineEdit::update() {
    Vector4 pos = Input::mousePosition();
    if(Input::touchCount() > 0) {
        pos = Input::touchPosition(0);
    }
    Vector4 color(m_normalColor);

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
            emitSignal(_SIGNAL(editingFinished()));
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
            emitSignal(_SIGNAL(focusOut()));
            m_focused = false;
            m_cursorVisible = false;
        }
    }

    bool hover = rectTransform()->isHovered(pos.x, pos.y);
    if(m_hovered != hover) {
        m_currentFade = 0.0f;
        m_hovered = hover;
    }

    if(m_hovered) {
        color = m_highlightedColor;

        if(!m_focused && (Input::isMouseButtonDown(0) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN))) {
            m_currentFade = 0.0f;

            Widget::setFocusWidget(this);

            emitSignal(_SIGNAL(focusIn()));
            m_focused = true;
        }

        if(Input::isMouseButtonUp(0)) {
            m_currentFade = 0.0f;
        }

        if(Input::isMouseButton(Input::MOUSE_LEFT) || Input::touchCount() > 0) {
            color = m_pressedColor;
        }
    }

    if(m_currentFade < 1.0f) {
        m_currentFade += 1.0f / m_fadeDuration * Timer::deltaTime();
        m_currentFade = CLAMP(m_currentFade, 0.0f, 1.0f);

        setColor(MIX(m_backgroundColor, color, m_currentFade));
    }

    Widget::update();
}
/*!
    \internal
    Overrides the composeComponent method to create the text input component.
*/
void LineEdit::composeComponent() {
    Widget::composeComponent();

    // Add label
    Actor *text = Engine::composeActor(gLabelClass, gText, actor());
    Label *label = text->getComponent<Label>();
    label->setAlign(Alignment::Middle | Alignment::Left);

    RectTransform *labelRect = label->rectTransform();
    labelRect->setSize(Vector2());
    labelRect->setAnchors(Vector2(0.0f), Vector2(1.0f));

    setSubWidget(label);
    setText("");

    RectTransform *rect = rectTransform();
    rect->setSize(Vector2(100.0f, 30.0f));
    rect->setPadding(Vector4(2.0f));

    setColor(m_normalColor);
    recalcCursor();
}
/*!
    \internal
    Recalculates the cursor position based on the current text and adjusts the label accordingly.
*/
void LineEdit::recalcCursor() {
    Label *label = static_cast<Label *>(subWidget(gText));
    if(label) {
        RectTransform *r = label->rectTransform();
        float pos = (m_cursorPosition > 0) ? label->cursorAt(m_cursorPosition).x : 0.0f;

        m_cursorTransform[12] = pos + rectTransform()->padding().w;
        m_cursorTransform[13] = rectTransform()->size().y * 0.5f;

        float x = r->position().x;
        float size = r->size().x;
        float gap = pos + x;
        float shift = 0.0f;
        if(gap > size) {
            shift = size - pos;
            r->setPosition(Vector3(shift, 0.0f, 0.0f));
        } else if(gap < 0.0f) {
            shift = x - gap;
            r->setPosition(Vector3(shift, 0.0f, 0.0f));
        }
        m_cursorTransform[12] += r->position().x;
    }
}
