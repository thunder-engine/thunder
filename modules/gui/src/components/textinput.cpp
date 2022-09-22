#include <components/textinput.h>

#include "components/frame.h"
#include "components/label.h"
#include "components/recttransform.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <resources/mesh.h>
#include <resources/font.h>

#include <input.h>
#include <timer.h>
#include <commandbuffer.h>
#include <utils.h>

namespace {
    const char *gLabel = "Label";
    const char *gFrame = "Frame";

    const float gCorner = 3.0f;
}

TextInput::TextInput() :
    m_normalColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f)),
    m_highlightedColor(Vector4(0.6f, 0.6f, 0.6f, 1.0f)),
    m_pressedColor(Vector4(0.7f, 0.7f, 0.7f, 1.0f)),
    m_textColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f)),
    m_background(nullptr),
    m_cursor(nullptr),
    m_label(nullptr),
    m_cursorPosition(0),
    m_fadeDuration(0.2f),
    m_currentFade(1.0f),
    m_cursorBlinkRate(0.85f),
    m_cursorBlinkCurrent(0.0f),
    m_hovered(false) {
}

Frame *TextInput::background() const {
    return m_background;
}
void TextInput::setBackground(Frame *frame) {
    if(m_background != frame) {
        disconnect(m_background, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
        m_background = frame;
        if(m_background) {
            connect(m_background, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            m_background->setColor(m_normalColor);
        }
    }
}

Label *TextInput::textComponent() const {
    return m_label;
}
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

string TextInput::text() const {
    if(m_label) {
        return m_label->text();
    }
    return string();
}
void TextInput::setText(const string text) {
    if(m_label) {
        m_label->setText(text);
    }
}

Vector4 TextInput::textColor() const {
    return m_textColor;
}
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
    \internal
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
            if(isBackspace) {
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
        } else {
            u32string sub = Utils::utf8ToUtf32(Input::inputString());
            sub.erase(std::remove(sub.begin(), sub.end(), "\r"), sub.end());
            sub.erase(std::remove(sub.begin(), sub.end(), "\n"), sub.end());
            if(!sub.empty()) {
                u32.insert(m_cursorPosition, sub);
                m_cursorPosition += sub.size();
                setText(Utils::utf32ToUtf8(u32));
                recalcCursor();
            }
        }
    } else {
        if(m_cursor) {
            m_cursor->setEnabled(false);
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
            onClicked();
        }

        if(Input::isMouseButtonUp(0)) {
            m_currentFade = 0.0f;
        }

        if(Input::isMouseButton(0) || Input::touchCount() > 0) {
            color = m_pressedColor;
        }
    }

    if(m_background) {
        if(m_currentFade < 1.0f) {
            m_currentFade += 1.0f / m_fadeDuration * Timer::deltaTime();
            m_currentFade = CLAMP(m_currentFade, 0.0f, 1.0f);

            m_background->setColor(MIX(m_background->color(), color, m_currentFade));
        }
    }

    Widget::update();
}

void TextInput::composeComponent() {
    Widget::composeComponent();

    // Add Background
    Frame *frame = Engine::objectCreate<Frame>(gFrame, actor());
    frame->setCorners(Vector4(gCorner));
    setBackground(frame);

    // Add label
    Actor *text = Engine::composeActor(gLabel, gLabel, actor());
    Label *label = static_cast<Label *>(text->component(gLabel));
    label->setAlign(Alignment::Top | Alignment::Left);
    label->rectTransform()->setOffsets(Vector2(gCorner, 0.0f), Vector2(gCorner, 0.0f));
    label->rectTransform()->setAnchors(Vector2(0.0f), Vector2(1.0f));
    setTextComponent(label);
    setText("");

    // Add cursor
    Actor *cursorActor = Engine::composeActor(gLabel, "Cursor", text);
    m_cursor = static_cast<Label *>(cursorActor->component(gLabel));
    m_cursor->setColor(m_textColor);
    m_cursor->setText("|");

    RectTransform *rect = m_cursor->rectTransform();
    float height = label->fontSize();
    rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));
    rect->setSize(Vector2(gCorner, height)); // gCorner should be replaced with width of cursor glyph
    rect->setPivot(Vector2(0.0f, 1.0f));

    rectTransform()->setSize(Vector2(100.0f, 30.0f));

    recalcCursor();
}
/*!
    \internal
*/
void TextInput::onReferenceDestroyed() {
    Object *object = sender();
    if(m_background == object) {
        m_background = nullptr;
        return;
    }
    if(m_label == object) {
        m_label = nullptr;
        return;
    }
}
/*!
    \internal
*/
void TextInput::onClicked() {
    setFocusWidget(this);

    if(m_cursor) {
        m_cursor->setEnabled(true);
    }

    emitSignal(_SIGNAL(clicked()));
}

void TextInput::recalcCursor() {
    if(m_label && m_cursor) {
        Vector2 pos = m_label->cursorAt(m_cursorPosition);
        m_cursor->rectTransform()->setPosition(Vector3(pos.x, -gCorner, 0.0f));

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
