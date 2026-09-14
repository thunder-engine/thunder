/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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
        repaint();
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
