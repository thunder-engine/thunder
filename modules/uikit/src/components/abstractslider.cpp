#include "components/abstractslider.h"

#include "components/recttransform.h"
#include "components/frame.h"

#include <input.h>

namespace {
    const char *gKnob("knob");

    const char *gFrameClass("Frame");
}

AbstractSlider::AbstractSlider() :
        m_value(2),
        m_minimum(0),
        m_maximum(10),
        m_currentFade(1.0f),
        m_orientation(Horizontal),
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
    if(m_value != value) {
        m_value = value;

        emitSignal(_SIGNAL(valueChanged), m_value);
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

void AbstractSlider::setKnob(Widget *handle) {
    setSubWidget(handle);
}

void AbstractSlider::update() {
}
