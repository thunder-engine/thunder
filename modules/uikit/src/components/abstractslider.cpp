#include "components/abstractslider.h"

#include "components/recttransform.h"

#include <timer.h>
#include <input.h>

namespace {
    const char *gKnob("knob");

    const float gFadeDuration = 0.2f;
}

/*!
    \class AbstractSlider
    \brief Abstract base class for slider-like widgets.
    \inmodule Gui

    AbstractSlider provides common functionality for widgets that allow
    selecting a value from a range using a draggable knob. This class
    is intended to be inherited by concrete slider implementations.
*/

AbstractSlider::AbstractSlider() :
        m_value(2),
        m_minimum(0),
        m_maximum(10),
        m_orientation(Widget::Horizontal),
        m_currentFade(1.0f),
        m_areaGap(0.0f),
        m_hovered(false) {

}
/*!
    Returns the current orientation of the slider (Horizontal or Vertical).
*/
int AbstractSlider::orientation() const {
    return m_orientation;
}
/*!
    Sets the \a orientation of the slider.
*/
void AbstractSlider::setOrientation(int orientation) {
    m_orientation = orientation;
    repaint();
}
/*!
    Returns the current value of the slider.
*/
int AbstractSlider::value() const {
    return m_value;
}
/*!
    Sets the current \a value of the slider.
*/
void AbstractSlider::setValue(int value) {
    value = CLAMP(value, m_minimum, m_maximum);
    if(m_value != value) {
        m_value = value;

        valueChanged(m_value);
    }
}
/*!
    Returns the minimum value of the slider.
*/
int AbstractSlider::minimum() const {
    return m_minimum;
}
/*!
    Sets the \a minimum value of the slider.
*/
void AbstractSlider::setMinimum(int minimum) {
    m_minimum = minimum;
}
/*!
    Returns the maximum value of the slider.
*/
int AbstractSlider::maximum() const {
    return m_maximum;
}
/*!
    Sets the \a maximum value of the slider.
*/
void AbstractSlider::setMaximum(int maximum) {
    m_maximum = maximum;
}
/*!
    Returns the knob widget.
*/
Widget *AbstractSlider::knob() const {
    return static_cast<Widget *>(subWidget(gKnob));
}
/*!
    Sets the \a knob widget.
*/
void AbstractSlider::setKnob(Widget *knob) {
    setSubWidget(knob);
}
/*!
    Emits the pressed() signal.

    Called when the slider is pressed (mouse down or touch begin).
*/
void AbstractSlider::pressed() {
    emitSignal(_SIGNAL(pressed()));
}
/*!
    Emits the valueChanged() signal.

    Called when the slider's \a value changes.
*/
void AbstractSlider::valueChanged(int value) {
    emitSignal(_SIGNAL(valueChanged(int)), value);
}
/*!
    \internal
*/
void AbstractSlider::update(const Vector2 &pos) {
    Widget::update(pos);

    bool hover = isHovered(pos);
    if(m_hovered != hover) {
        m_currentFade = 0.0f;
        m_hovered = hover;
    }

    if(m_hovered) {
        if(Input::isMouseButtonDown(Input::MOUSE_LEFT) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN)) {
            m_currentFade = 0.0f;

            Widget::setFocusWidget(this);

            pressed();
        }

        if((Input::isMouseButton(Input::MOUSE_LEFT) || Input::touchCount() > 0)) {
            Vector3 min, max;
            rectTransform()->bound().box(min, max);
            if(m_orientation == Widget::Horizontal) {
                min.x += m_areaGap;
                max.x -= m_areaGap;
            } else {
                min.y += m_areaGap;
                max.y -= m_areaGap;
            }

            Vector2 local(pos - min);
            Vector2 factor(local.x / (max.x - min.x), local.y / (max.y - min.y));

            float f = (m_orientation == Widget::Horizontal) ? factor.x : (m_areaGap > 0.0f ? 1.0f - factor.y : factor.y);
            if(f >= 0.0f && f <= 1.0f) {
                int newValue = round(MIX(m_minimum, m_maximum, f));
                if(newValue != m_value) {
                    setValue(newValue);

                    valueChanged(newValue);
                }
            }
        }
    }

    if(m_currentFade < 1.0f) {
        m_currentFade += 1.0f / gFadeDuration * Timer::deltaTime();
        m_currentFade = CLAMP(m_currentFade, 0.0f, 1.0f);
        repaint();
    }
}
