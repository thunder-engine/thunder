#include "components/gui/textinput.h"

#include "components/gui/label.h"
#include "components/gui/frame.h"
#include "components/gui/recttransform.h"

#include "components/actor.h"
#include "components/textrender.h"

#include "resources/mesh.h"
#include "resources/font.h"

#include "input.h"
#include "timer.h"
#include "utils.h"

#include <algorithm>

namespace {
    const char *gLabel = "Label";
    const char *gFrame = "Frame";
    const char *gBackground = "Background";
    const char *gCursor = "Cursor";
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
        m_cursor(nullptr),
        m_label(nullptr),
        m_background(nullptr),
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
string TextInput::text() const {
    if(m_label) {
        return m_label->text();
    }

    return string();
}
/*!
    Sets the \a text in the TextInput.
*/
void TextInput::setText(const string text) {
    if(m_label) {
        m_label->setText(text);
        u32string u32 = Utils::utf8ToUtf32(text);
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
    if(m_label) {
        m_label->setColor(m_textColor);
    }

    if(m_cursor) {
        m_cursor->setColor(m_textColor);
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
    if(m_background) {
        m_background->setColor(m_normalColor);
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
    return m_label;
}
/*!
    Sets the text \a label component.
*/
void TextInput::setTextComponent(Label *label) {
    if(m_label != label) {
        disconnect(m_label, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        m_label = label;
        if(m_label) {
            connect(m_label, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            setTextColor(m_textColor);
        }
    }
}
/*!
    Returns the background frame component.
*/
Frame *TextInput::background() const {
    return m_background;
}
/*!
    Sets the background \a frame component.
*/
void TextInput::TextInput::setBackground(Frame *frame) {
    if(m_background != frame) {
        disconnect(m_background, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        m_background = frame;
        if(m_background) {
            connect(m_background, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        }
    }
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
            m_cursor->setEnabled(!m_cursor->isEnabled());
        }

        u32string u32 = Utils::utf8ToUtf32(text());
        bool isBackspace = Input::isKeyDown(Input::KEY_BACKSPACE);
        if(isBackspace || Input::isKeyDown(Input::KEY_DELETE)) {
            if(isBackspace && m_cursorPosition >= 0) {
                m_cursorPosition--;
            }
            if(m_cursorPosition >= 0) {
                u32.erase(m_cursorPosition, 1);
                setText(Utils::utf32ToUtf8(u32));
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
            string sub = Input::inputString();
            sub.erase(remove_if(sub.begin(), sub.end(), [](unsigned char c) { return (c < 32);}), sub.end());
            if(!sub.empty()) {
                u32string u32sub = Utils::utf8ToUtf32(sub);
                u32.insert(m_cursorPosition, u32sub);
                m_cursorPosition += u32sub.size();
                setText(Utils::utf32ToUtf8(u32));
                recalcCursor();
            }
        }
    } else {
        if(m_cursor) {
            m_cursor->setEnabled(false);
        }

        if(m_focused) {
            emitSignal(_SIGNAL(focusOut()));
            m_focused = false;
        }
    }

    bool hover = rectTransform()->isHovered(pos.x, pos.y);
    if(m_hovered != hover) {
        m_currentFade = 0.0f;
        m_hovered = hover;
    }

    if(m_hovered) {
        color = m_highlightedColor;
        if(Input::isMouseButtonDown(0) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN)) {
            m_currentFade = 0.0f;

            Widget::setFocusWidget(this);

            if(m_cursor) {
                m_cursor->setEnabled(true);
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

        m_background->setColor(MIX(m_background->color(), color, m_currentFade));
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
    Actor *background = Engine::composeActor(gFrame, gBackground, actor());
    Frame *backgroundFrame = static_cast<Frame *>(background->component(gFrame));
    backgroundFrame->setColor(m_normalColor);
    backgroundFrame->makeInternal();
    backgroundFrame->rectTransform()->setAnchors(Vector2(0.0f), Vector2(1.0f));
    setBackground(backgroundFrame);

    // Add label
    Actor *text = Engine::composeActor(gLabel, gLabel, background);
    Label *label = static_cast<Label *>(text->component(gLabel));
    label->setAlign(Alignment::Middle | Alignment::Left);
    label->makeInternal();
    float corners = backgroundFrame->corners().x;
    label->rectTransform()->setMargin(Vector4(0.0, corners, 0.0f, corners));
    label->rectTransform()->setAnchors(Vector2(0.0f), Vector2(1.0f));
    setTextComponent(label);
    setText("");

    // Add cursor
    Actor *cursorActor = Engine::composeActor(gLabel, gCursor, text);
    m_cursor = static_cast<Label *>(cursorActor->component(gLabel));
    m_cursor->setColor(m_textColor);
    m_cursor->makeInternal();
    m_cursor->setText("|");

    RectTransform *rect = m_cursor->rectTransform();
    float height = label->fontSize();
    rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));
    rect->setSize(Vector2(corners, height)); // corners should be replaced with width of cursor glyph
    rect->setPivot(Vector2(0.0f, 1.0f));

    rectTransform()->setSize(Vector2(100.0f, 30.0f));

    recalcCursor();
}
/*!
    \internal
    Overrides the onReferenceDestroyed method to handle label destruction.
*/
void TextInput::onReferenceDestroyed() {
    Object *object = sender();
    if(m_label == object) {
        m_label = nullptr;
    } else if(m_background == object) {
        m_background = nullptr;
    }
}
/*!
    \internal
    Recalculates the cursor position based on the current text and adjusts the label accordingly.
*/
void TextInput::recalcCursor() {
    if(m_label && m_cursor) {
        Vector2 pos = m_label->cursorAt(m_cursorPosition);
        float corner = 0.0f;
        if(m_background) {
            corner = m_background->corners().x;
        }

        m_cursor->rectTransform()->setPosition(Vector3(pos.x, -corner, 0.0f));

        float x = m_label->rectTransform()->position().x;
        float size = m_label->rectTransform()->size().x;
        float gap = pos.x + x;
        if(gap > size) {
            float shift = size - pos.x;
            m_label->rectTransform()->setPosition(Vector3(shift, 0.0f, 0.0f));
            m_label->setClipOffset(Vector2(-shift, 0.0f));
        } else if(gap < 0.0f) {
            float shift = x - gap;
            m_label->rectTransform()->setPosition(Vector3(shift, 0.0f, 0.0f));
            m_label->setClipOffset(Vector2(-shift, 0.0f));
        }
    }
}
