#include "components/textinput.h"

#include "components/label.h"
#include "components/frame.h"
#include "components/recttransform.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <resources/mesh.h>
#include <resources/font.h>

#include <input.h>
#include <timer.h>

#include <algorithm>

namespace {
    const char *gBackground("background");
    const char *gCursor("cursor");
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

TextInput::TextInput() :
        m_normalColor(0.5f, 0.5f, 0.5f, 1.0f),
        m_highlightedColor(0.6f, 0.6f, 0.6f, 1.0f),
        m_pressedColor(0.7f, 0.7f, 0.7f, 1.0f),
        m_textColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_cursorPosition(0),
        m_fadeDuration(0.2f),
        m_currentFade(1.0f),
        m_cursorBlinkRate(0.85f),
        m_cursorBlinkCurrent(0.0f),
        m_hovered(false),
        m_focused(false) {
}
/*!
    Returns the current text entered into the TextInput.
*/
TString TextInput::text() const {
    Label *label = TextInput::textComponent();
    if(label) {
        return label->text();
    }

    return TString();
}
/*!
    Sets the \a text in the TextInput.
*/
void TextInput::setText(const TString text) {
    Label *label = TextInput::textComponent();
    if(label) {
        label->setText(text);
        std::u32string u32 = text.toUtf32();
        m_cursorPosition = u32.size();
        recalcCursor();
    }
}
/*!
    Returns the color of the text.
*/
Vector4 TextInput::textColor() const {
    return m_textColor;
}
/*!
    Sets the \a color of the text.
*/
void TextInput::setTextColor(Vector4 color) {
    m_textColor = color;

    Label *label = TextInput::textComponent();
    if(label) {
        label->setColor(m_textColor);
    }

    Label *cursor = TextInput::textCursor();
    if(cursor) {
        cursor->setColor(m_textColor);
    }
}
/*!
    Returns the color of the background.
*/
Vector4 TextInput::backgroundColor() const {
    return m_normalColor;
}
/*!
    Sets the \a color of the background.
*/
void TextInput::setBackgroundColor(Vector4 color) {
    m_normalColor = color;

    Frame *background = TextInput::background();
    if(background) {
        background->setColor(m_normalColor);
    }
}
/*!
    Returns the color of the background in hover state.
*/
Vector4 TextInput::hoverColor() const {
    return m_highlightedColor;
}
/*!
    Sets the \a color of the background in hover state.
*/
void TextInput::setHoverColor(Vector4 color) {
    m_highlightedColor = color;
}
/*!
    Returns the color of the background in pressed state.
*/
Vector4 TextInput::pressedColor() const {
    return m_pressedColor;
}
/*!
    Sets the \a color of the background in pressed state.
*/
void TextInput::setPressedColor(Vector4 color) {
    m_pressedColor = color;
}
/*!
    Returns the text label component.
*/
Label *TextInput::textComponent() const {
    return static_cast<Label *>(subWidget(gText));
}
/*!
    Sets the text \a label component.
*/
void TextInput::setTextComponent(Label *label) {
    setSubWidget(gText, label);
}
/*!
    Returns the text cursor component.
*/
Label *TextInput::textCursor() const {
    return static_cast<Label *>(subWidget(gCursor));
}
/*!
    Sets the text \a cursor component.
*/
void TextInput::setTextCursor(Label *cursor) {
    setSubWidget(gCursor, cursor);
}
/*!
    Returns the background frame component.
*/
Frame *TextInput::background() const {
    return static_cast<Frame *>(subWidget(gBackground));
}
/*!
    Sets the background \a frame component.
*/
void TextInput::TextInput::setBackground(Frame *frame) {
    setSubWidget(gBackground, frame);
}
/*!
    \internal
    Overrides the update method to handle text input and cursor animation.
*/
void TextInput::update() {
    Vector4 pos = Input::mousePosition();
    if(Input::touchCount() > 0) {
        pos = Input::touchPosition(0);
    }
    Vector4 color(m_normalColor);

    if(Widget::focusWidget() == this) {
        m_cursorBlinkCurrent += Timer::deltaTime();
        if(m_cursorBlinkCurrent >= m_cursorBlinkRate) {
            m_cursorBlinkCurrent = 0.0f;
            Label *cursor = TextInput::textCursor();
            if(cursor) {
                cursor->setEnabled(!cursor->isEnabled());
            }
        }

        std::u32string u32 = text().toUtf32();
        bool isBackspace = Input::isKeyDown(Input::KEY_BACKSPACE);
        if(isBackspace || Input::isKeyDown(Input::KEY_DELETE)) {
            if(isBackspace && m_cursorPosition >= 0) {
                m_cursorPosition--;
            }
            if(m_cursorPosition >= 0) {
                u32.erase(m_cursorPosition, 1);
                setText(TString::fromUtf32(u32));
            } else {
                m_cursorPosition = 0;
            }
            recalcCursor();
        } else if(Input::isKeyDown(Input::KEY_LEFT) && m_cursorPosition > 0) {
            m_cursorPosition--;
            recalcCursor();
        } else if(Input::isKeyDown(Input::KEY_RIGHT) && m_cursorPosition < u32.size()) {
            m_cursorPosition++;
            recalcCursor();
        } else if(Input::isKeyDown(Input::KEY_ENTER) || Input::isKeyDown(Input::KEY_KP_ENTER)) {
            emitSignal(_SIGNAL(editingFinished()));
        } else {
            TString sub = Input::inputString();
            std::string s = sub.toStdString();
            s.erase(remove_if(s.begin(), s.end(), [](unsigned char c) { return (c < 32);}), s.end());
            sub = s;
            if(!sub.isEmpty()) {
                std::u32string u32sub = sub.toUtf32();
                u32.insert(m_cursorPosition, u32sub);
                m_cursorPosition += u32sub.size();
                setText(TString::fromUtf32(u32));
                recalcCursor();
            }
        }
    } else {
        if(m_focused) {
            emitSignal(_SIGNAL(focusOut()));
            m_focused = false;

            Label *cursor = TextInput::textCursor();
            if(cursor) {
                cursor->setEnabled(false);
            }
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

            Label *cursor = TextInput::textCursor();
            if(cursor) {
                cursor->setEnabled(true);
            }
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

        Frame *background = TextInput::background();
        if(background) {
            background->setColor(MIX(background->color(), color, m_currentFade));
        }
    }

    Widget::update();
}
/*!
    \internal
    Overrides the composeComponent method to create the text input component.
*/
void TextInput::composeComponent() {
    Widget::composeComponent();

    // Add Background
    Actor *background = Engine::composeActor(gFrameClass, gBackground, actor());
    Frame *backgroundFrame = background->getComponent<Frame>();
    backgroundFrame->setColor(m_normalColor);
    backgroundFrame->rectTransform()->setAnchors(Vector2(0.0f), Vector2(1.0f));

    setBackground(backgroundFrame);

    // Add label
    Actor *text = Engine::composeActor(gLabelClass, gLabelClass, background);
    Label *label = text->getComponent<Label>();
    label->setAlign(Alignment::Middle | Alignment::Left);
    float corners = backgroundFrame->corners().x;

    RectTransform *labelRect = label->rectTransform();
    labelRect->setSize(Vector2());
    labelRect->setMargin(Vector4(0.0, corners, 0.0f, corners));
    labelRect->setAnchors(Vector2(0.0f), Vector2(1.0f));

    setTextComponent(label);
    setText("");

    // Add cursor
    Actor *cursorActor = Engine::composeActor(gLabelClass, gLabelClass, text);
    Label *cursor = cursorActor->getComponent<Label>();
    cursor->setAlign(Alignment::Middle | Alignment::Left);
    cursor->setColor(m_textColor);
    cursor->setText("|");

    setTextCursor(cursor);

    RectTransform *rect = cursor->rectTransform();
    rect->setSize(Vector2(corners, 0)); // corners should be replaced with width of cursor glyph
    rect->setAnchors(Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f));

    rectTransform()->setSize(Vector2(100.0f, 30.0f));

    recalcCursor();
}
/*!
    \internal
    Recalculates the cursor position based on the current text and adjusts the label accordingly.
*/
void TextInput::recalcCursor() {
    Label *cursor = TextInput::textCursor();
    Label *label = TextInput::textComponent();
    if(label && cursor) {
        Vector2 pos = label->cursorAt(m_cursorPosition);
        float corner = 0.0f;

        Frame *background = TextInput::background();
        if(background) {
            corner = background->corners().x;
        }

        cursor->rectTransform()->setPosition(Vector3(pos.x, -corner, 0.0f));

        float x = label->rectTransform()->position().x;
        float size = label->rectTransform()->size().x;
        float gap = pos.x + x;
        if(gap > size) {
            float shift = size - pos.x;
            label->rectTransform()->setPosition(Vector3(shift, 0.0f, 0.0f));
            label->setClipOffset(Vector2(-shift, 0.0f));
        } else if(gap < 0.0f) {
            float shift = x - gap;
            label->rectTransform()->setPosition(Vector3(shift, 0.0f, 0.0f));
            label->setClipOffset(Vector2(-shift, 0.0f));
        }
    }
}
