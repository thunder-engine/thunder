#include "components/abstractbutton.h"

#include "components/recttransform.h"
#include "components/canvas.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <resources/stylesheet.h>

#include <input.h>
#include <timer.h>
#include <log.h>

namespace {
    const char *gBackgroundColor("backgroundColor");
    const char *gColor("mainColor");
}
/*!
    \class AbstractButton
    \brief The AbstractButton class represents a base class for interactive buttons in a graphical user interface.
    \inmodule Gui

    The AbstractButton class provides a foundation for creating interactive buttons within a graphical user interface.
    It allows customization of various visual properties and handles user interaction events.
    Internal methods are marked as internal and are intended for use within the framework rather than by external code.
*/

AbstractButton::AbstractButton() :
        m_highlightedColor(Vector4(0.6f, 0.6f, 0.6f, 1.0f)),
        m_pressedColor(Vector4(0.7f, 0.7f, 0.7f, 1.0f)),
        m_currentColor(m_backgroundColor),
        m_fadeDuration(0.1f),
        m_currentFade(1.0f),
        m_hovered(false),
        m_checkable(false),
        m_checked(false),
        m_exclusive(false) {
}

AbstractButton::~AbstractButton() {
    delete m_imageMaterial;
    delete m_frameMaterial;

    delete m_backgroundMesh;
}
/*!
    \internal
*/
void AbstractButton::setHovered(bool hover, bool instant) {
    if(m_hovered != hover) {
        if(!hover && instant) {
            updateBackgroundColor(m_checked ? m_pressedColor : m_backgroundColor);
        } else {
            m_currentFade = 0.0f;
        }
        m_hovered = hover;
    }
}
/*!
    Returns the color used when the button is highlighted.
*/
Vector4 AbstractButton::highlightedColor() const {
    return m_highlightedColor;
}
/*!
    Sets the \a color used when the button is highlighted.
*/
void AbstractButton::setHighlightedColor(const Vector4 &color) {
    m_highlightedColor = color;
}
/*!
    Returns the color used when the button is pressed.
*/
Vector4 AbstractButton::pressedColor() const {
    return m_pressedColor;
}
/*!
    Sets the \a color used when the button is pressed.
*/
void AbstractButton::setPressedColor(const Vector4 &color) {
    m_pressedColor = color;
}
/*!
    Returns true if the button is checkable; otherwise, false.
*/
bool AbstractButton::isCheckable() const {
    return m_checkable;
}
/*!
    Sets whether the button is \a checkable.
*/
void AbstractButton::setCheckable(bool checkable) {
    m_checkable = checkable;
}
/*!
    Returns true if the button is checked; otherwise, false.
*/
bool AbstractButton::isChecked() const {
    return m_checked;
}
/*!
    Sets the \a checked state of the button.
*/
void AbstractButton::setChecked(bool checked) {
    m_checked = checked;

    updateBackgroundColor(checked ? m_pressedColor : m_backgroundColor);

    checkStateSet();

    toggled(m_checked);
}
/*!
    Returns true if the button is in exclusive mode; otherwise, false.
*/
bool AbstractButton::isExclusive() const {
    return m_exclusive;
}
/*!
    Sets whether the button is in \a exclusive mode.
*/
void AbstractButton::setExclusive(bool exclusive) {
    m_exclusive = exclusive;
}

void AbstractButton::pressed() {
    emitSignal(_SIGNAL(pressed()));
}

void AbstractButton::clicked() {
    emitSignal(_SIGNAL(clicked()));
}

void AbstractButton::toggled(bool checked) {
    emitSignal(_SIGNAL(toggled(bool)), checked);
}
/*!
    \internal
    Internal method called to update the button's visual appearance and handle user interaction.
*/
void AbstractButton::update(const Vector2 &pos) {
    Widget::update(pos);

    setHovered(isHovered(pos));

    Vector4 color(m_checked ? m_pressedColor : m_backgroundColor);
    if(m_hovered) {
        color = m_highlightedColor;
        if(Input::isMouseButtonDown(Input::MOUSE_LEFT) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN)) {
            m_currentFade = 0.0f;

            if(m_checkable) {
                setChecked(!m_checked);
            }
            pressed();
        }

        if(Input::isMouseButtonUp(Input::MOUSE_LEFT) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_ENDED)) {
            m_currentFade = 0.0f;

            clicked();
        }

        if(Input::isMouseButton(Input::MOUSE_LEFT) || Input::touchCount() > 0) {
            color = m_pressedColor;
        }
    }

    if(m_currentFade < 1.0f) {
        m_currentFade += 1.0f / m_fadeDuration * Timer::deltaTime();
        m_currentFade = CLAMP(m_currentFade, 0.0f, 1.0f);

        updateBackgroundColor(MIX(m_currentColor, color, m_currentFade));
    }
}
/*!
    \internal
    Internal method to handle the button's checked state, ensuring exclusivity in exclusive mode.
*/
void AbstractButton::checkStateSet() {
    if(m_exclusive && m_checked) {
        for(auto it : actor()->getChildren()) {
            Actor *actor = dynamic_cast<Actor *>(it);
            if(actor) {
                AbstractButton *btn = actor->getComponent<AbstractButton>();
                if(btn && btn != this) {
                    btn->setChecked(false);
                }
            }
        }
    }
}

void AbstractButton::updateBackgroundColor(const Vector4 &color) {
    m_currentColor = color;
    if(m_backgroundImage) {
        m_imageMaterial->setVector4(gColor, &color);
    } else {
        m_frameMaterial->setVector4(gBackgroundColor, &color);
    }
}
