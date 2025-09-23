#include "components/abstractslider.h"

#include "components/recttransform.h"
#include "components/frame.h"

#include <timer.h>
#include <input.h>

namespace {
    const char *gKnob("knob");

    const char *gFrameClass("Frame");

    const float gFadeDuration = 0.2f;
}

AbstractSlider::AbstractSlider() :
        m_value(2),
        m_minimum(0),
        m_maximum(10),
        m_orientation(Horizontal),
        m_currentFade(1.0f),
        m_areaGap(0.0f),
        m_hovered(false) {

}

int AbstractSlider::orientation() const {
    return m_orientation;
}

void AbstractSlider::setOrientation(int orientation) {
    m_orientation = orientation;
}

int AbstractSlider::value() const {
    return m_value;
}

void AbstractSlider::setValue(int value) {
    value = CLAMP(value, m_minimum, m_maximum);
    if(m_value != value) {
        m_value = value;

        valueChanged(m_value);
    }
}

int AbstractSlider::minimum() const {
    return m_minimum;
}

void AbstractSlider::setMinimum(int minimum) {
    m_minimum = minimum;
}

int AbstractSlider::maximum() const {
    return m_maximum;
}

void AbstractSlider::setMaximum(int maximum) {
    m_maximum = maximum;
}

Widget *AbstractSlider::knob() const {
    return static_cast<Widget *>(subWidget(gKnob));
}

void AbstractSlider::setKnob(Widget *knob) {
    setSubWidget(knob);
}

void AbstractSlider::pressed() {
    emitSignal(_SIGNAL(pressed()));
}

void AbstractSlider::valueChanged(int value) {
    emitSignal(_SIGNAL(valueChanged(int)), value);
}

void AbstractSlider::update() {
    Vector4 pos = Input::mousePosition();
    if(Input::touchCount() > 0) {
        pos = Input::touchPosition(0);
    }

    RectTransform *rect = rectTransform();

    bool hover = rect->isHovered(pos.x, pos.y);
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
            rect->bound().box(min, max);
            if(m_orientation == Horizontal) {
                min.x += m_areaGap;
                max.x -= m_areaGap;
            } else {
                min.y += m_areaGap;
                max.y -= m_areaGap;
            }

            Vector2 local(Vector2(pos.x, pos.y) - min);
            Vector2 factor(local.x / (max.x - min.x), local.y / (max.y - min.y));

            float f = (m_orientation == Horizontal) ? factor.x : (m_areaGap > 0.0f ? 1.0f - factor.y : factor.y);
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
    }
}
