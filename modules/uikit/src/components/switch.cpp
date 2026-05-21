#include "components/switch.h"

#include "components/recttransform.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <timer.h>

/*!
    \class Switch
    \brief The Switch class is a UI component that acts as a switch or toggle button with a graphical knob.
    \inmodule Gui

    The Switch class provides a customizable switch button with an animated graphical knob.
    It inherits functionality from the AbstractButton class and extends it to handle knob-related features and animations.
*/

Switch::Switch() :
        CheckBox(),
        m_switchDuration(0.2f),
        m_currentFade(1.0f) {

    m_switchMode = true;
}
/*!
    \internal
    Overrides the update method to handle knob animation.
*/
void Switch::update(const Vector2 &pos) {
    AbstractButton::update(pos);

    if(m_currentFade < 1.0f) {
        m_currentFade += 1.0f / m_switchDuration * Timer::deltaTime();
        m_currentFade = CLAMP(m_currentFade, 0.0f, 1.0f);

        float width = rectTransform()->size().x;
        float left = (width - m_knobSize.x) * -0.5f;
        float right = (width - m_knobSize.x) * 0.5f;
        m_iconOffset = m_checked ? MIX(left, right, m_currentFade) : MIX(right, left, m_currentFade);
    }
}
/*!
    \internal
    Overrides the checkStateSet method to handle state changes.
*/
void Switch::checkStateSet() {
    AbstractButton::checkStateSet();
    m_currentFade = 0.0f;
}
/*!
    \internal
*/
void Switch::composeComponent() {
    CheckBox::composeComponent();

    RectTransform *rect = rectTransform();
    rect->blockSignals(true);
    rect->setSize(Vector2(32.0f, 16.0f));
    rect->blockSignals(false);

    blockSignals(true);
    setIndicatorSize(Vector2(16.0f));
    blockSignals(false);
}
/*!
    \internal
*/
void Switch::boundChanged(const Vector2 &size) {
    CheckBox::boundChanged(size);

    if(m_checked) {
        m_iconOffset = (size.x - m_knobSize.x) * 0.5f;
    } else {
        m_iconOffset = (size.x - m_knobSize.x) *-0.5f;
    }
}
