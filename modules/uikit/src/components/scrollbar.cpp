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
#include "scrollbar.h"

#include "frame.h"
#include "button.h"

namespace {
    const char *gKnob("knob");
    const char *gBackArrow("backArrow");
    const char *gFrontArrow("frontArrow");

    const char *gSpriteArrow(".embedded/ui.png/Arrow");
}

/*!
    \class ScrollBar
    \brief The ScrollBar class provides a scroll bar widget that allows users to navigate through a range of values.
    \inmodule Gui

    ScrollBar provides a typical scroll bar with arrow buttons and a draggable knob.
    Supports both horizontal and vertical orientations.
*/

ScrollBar::ScrollBar() :
        m_pageStep(2),
        m_singleStep(1) {

    m_areaGap = 16.0f;
}
/*!
    Sets the current \a value of the scroll bar.
*/
void ScrollBar::setValue(int value) {
    AbstractSlider::setValue(value);

    recalcKnob();
}
/*!
    Sets the minimum \a value of the scroll bar.
*/
void ScrollBar::setMinimum(int value) {
    AbstractSlider::setMinimum(value);

    recalcKnob();
}
/*!
    Sets the maximum \a value of the scroll bar.
*/
void ScrollBar::setMaximum(int value) {
    AbstractSlider::setMaximum(value);

    recalcKnob();
}
/*!
    Sets the \a orientation of the scroll bar.
*/
void ScrollBar::setOrientation(int orientation) {
    AbstractSlider::setOrientation(orientation);

    setBackArrow(backArrow());
    setFrontArrow(frontArrow());

    recalcKnob();
}
/*!
    Returns the page step size.
    The page step determines how much the value changes when clicking
*/
int ScrollBar::pageStep() const {
    return m_pageStep;
}
/*!
    Sets the \a page step size.
    The page step determines how much the value changes when clicking on the track area.
*/
void ScrollBar::setPageStep(int page) {
    m_pageStep = page;

    recalcKnob();
}
/*!
    Returns the single step size.
*/
int ScrollBar::singleStep() const {
    return m_singleStep;
}
/*!
    Sets the single \a step size.
    The single step determines how much the value changes when clicking the arrow buttons.
*/
void ScrollBar::setSingleStep(int step) {
    m_singleStep = step;
}
/*!
    Moves the value backward by one single step.
*/
void ScrollBar::stepBack() {
    setValue(m_value - m_singleStep);
}
/*!
    Moves the value forward by one single step.
*/
void ScrollBar::stepFront() {
    setValue(m_value + m_singleStep);
}
/*!
    Returns the back arrow widget.
*/
Widget *ScrollBar::backArrow() const {
    return subWidget(gBackArrow);
}
/*!
    Sets the back \a arrow widget.
*/
void ScrollBar::setBackArrow(Widget *arrow) {
    Button *button = dynamic_cast<Button *>(backArrow());
    if(button) {
        disconnect(button, _SIGNAL(pressed()), this, _SLOT(stepBack()));
    }

    setSubWidget(arrow);

    if(arrow) {
        Widget *rect = arrow;
        if(m_orientation == Widget::Horizontal) {
            rect->setPivot(Vector2(0.0f, 0.5f));
            rect->setAnchors(Vector2(0.0f, 0.0f), Vector2(0.0, 1.0f));
            rect->setSize(Vector2(m_areaGap, 0.0f));
        } else {
            rect->setPivot(Vector2(0.5f, 1.0f));
            rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0, 1.0f));
            rect->setSize(Vector2(0.0f, m_areaGap));
        }
    }

    button = dynamic_cast<Button *>(arrow);
    if(button) {
        connect(button, _SIGNAL(pressed()), this, _SLOT(stepBack()));

        if(m_orientation == Widget::Horizontal) {
            button->setIconRotation(-90.0f);
        } else {
            button->setIconRotation(180.0f);
        }
    }
}
/*!
    Returns the front arrow widget.
*/
Widget *ScrollBar::frontArrow() const {
    return subWidget(gFrontArrow);
}
/*!
    Sets the front \a arrow widget.
*/
void ScrollBar::setFrontArrow(Widget *arrow) {
    Button *button = dynamic_cast<Button *>(frontArrow());
    if(button) {
        disconnect(button, _SIGNAL(pressed()), this, _SLOT(stepFront()));
    }

    setSubWidget(arrow);

    if(arrow) {
        Widget *rect = arrow;
        if(m_orientation == Widget::Horizontal) {
            rect->setPivot(Vector2(1.0f, 0.5f));
            rect->setAnchors(Vector2(1.0f, 0.0f), Vector2(1.0, 1.0f));
            rect->setSize(Vector2(m_areaGap, 0.0f));
        } else {
            rect->setPivot(Vector2(0.5f, 0.0f));
            rect->setAnchors(Vector2(0.0f, 0.0f), Vector2(1.0, 0.0f));
            rect->setSize(Vector2(0.0f, m_areaGap));
        }
    }

    button = dynamic_cast<Button *>(arrow);
    if(button) {
        connect(button, _SIGNAL(pressed()), this, _SLOT(stepFront()));
        if(m_orientation == Widget::Horizontal) {
            button->setIconRotation(90.0f);
        } else {
            button->setIconRotation(0.0f);
        }
    }
}
/*!
    \internal
*/
void ScrollBar::recalcKnob() {
    Widget *knob = ScrollBar::knob();
    if(knob) {
        Widget *rect = this;
        Vector2 size(rect->size());

        int scrollingRange = m_maximum - m_minimum;

        float pageFactor = 1.0f;
        if (scrollingRange != 0 && m_pageStep != 0) {
            pageFactor = CLAMP(static_cast<float>(m_pageStep) / static_cast<float>(scrollingRange + m_pageStep), 0.0f, 1.0f);
        }

        float factor = m_value != 0 ? static_cast<float>(m_value - m_minimum) / static_cast<float>(scrollingRange) : 0.0f;
        factor = CLAMP(factor * (1.0 - pageFactor), 0.0f, 1.0f);

        Widget *knobRect = knob;
        if(m_orientation == Widget::Horizontal) {
            float normalGap = m_areaGap / size.x;
            float scaledFactor = normalGap + factor * (1.0f - normalGap * 2.0f);
            float scaledPage = pageFactor * (1.0f - normalGap * 2.0f);
            knobRect->setPivot(Vector2(0.0f, 0.5f));
            knobRect->setAnchors(Vector2(scaledFactor, 0.5f), Vector2(scaledFactor, 0.5f));
            knobRect->setSize(Vector2(MIX(0.0f, size.x, scaledPage), 8.0f));
        } else {
            float normalGap = m_areaGap / size.y;
            float scaledFactor = normalGap + factor * (1.0f - normalGap * 2.0f);
            float scaledPage = pageFactor * (1.0f - normalGap * 2.0f);
            knobRect->setPivot(Vector2(0.5f, 1.0f));
            knobRect->setAnchors(Vector2(0.5f, 1.0f - scaledFactor), Vector2(0.5f, 1.0f - scaledFactor));
            knobRect->setSize(Vector2(8.0f, MIX(0.0f, size.y, scaledPage)));
        }
    }
    repaint();
}
/*!
    \internal
*/
void ScrollBar::boundChanged(const Vector2 &size) {
    AbstractSlider::boundChanged(size);

    recalcKnob();
}
/*!
    \internal
*/
void ScrollBar::update(const Vector2 &pos) {
    AbstractSlider::update(pos);
}
/*!
    \internal
*/
void ScrollBar::composeComponent() {
    AbstractSlider::composeComponent();

    // Add knob
    Widget *knobActor = Widget::createWidget<Frame>(gKnob, this);
    Frame *knob = dynamic_cast<Frame *>(knobActor);
    knob->setCorners(Vector4(5.0f));
    knob->setBackgroundColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f));

    Widget *rectKnob = knob;

    if(m_orientation == Widget::Horizontal) {
        rectKnob->setAnchors(Vector2(0.0f, 0.5f), Vector2(0.0f, 0.5f));
    } else {
        rectKnob->setAnchors(Vector2(0.5f, 0.0f), Vector2(0.5f, 0.0f));
    }
    setKnob(knob);

    // Add back arrow
    Widget *backActor = Widget::createWidget<Button>(gBackArrow, this);
    Button *back = dynamic_cast<Button *>(backActor);
    back->setIconSize(Vector2(16.0f, 8.0f));
    back->setIcon(Engine::loadResource<Sprite>(gSpriteArrow));
    setBackArrow(back);

    // Add front arrow
    Widget *frontActor = Widget::createWidget<Button>(gFrontArrow, this);
    Button *front = dynamic_cast<Button *>(frontActor);
    front->setIconSize(Vector2(16.0f, 8.0f));
    front->setIcon(Engine::loadResource<Sprite>(gSpriteArrow));
    setFrontArrow(front);

    setValue(m_value);

    if(m_orientation == Widget::Horizontal) {
        this->setSize(Vector2(100.0f, 20.0f));
    } else {
        this->setSize(Vector2(20.0f, 100.0f));
    }
}
