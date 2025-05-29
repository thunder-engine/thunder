#include "components/floatinput.h"

#include "components/layout.h"
#include "components/recttransform.h"
#include "components/button.h"
#include "components/textinput.h"
#include "components/label.h"
#include "components/frame.h"
#include "components/image.h"

#include <components/actor.h>

#include <resources/sprite.h>

#include <sstream>

namespace {
    const char *gIncrease("increase");
    const char *gDecrease("decrease");
    const char *gInput("input");

    const char *gTextInputClass("TextInput");
    const char *gButtonClass("Button");
};

/*!
    \class FloatInput
    \brief The FloatInput class represents a user interface element for entering and displaying floating-point values.
    \inmodule Gui

    The FloatInput class represents a user interface element designed for entering and displaying floating-point values.
    This class is used in graphical user interface (GUI) applications where users need to input decimal numbers, such as 3.14, 0.001, or -42.56.
*/

FloatInput::FloatInput() :
        m_value(0.0f),
        m_singleStep(1.0f),
        m_minimum(0.0f),
        m_maximum(99.99f) {
}
/*!
    Returns the current value of the FloatInput.
*/
float FloatInput::value() const {
    return m_value;
}
/*!
    Sets the \a value of the FloatInput within the specified minimum and maximum limits.
*/
void FloatInput::setValue(float value) {
    m_value = CLAMP(value, m_minimum, m_maximum);

    std::ostringstream ss;
    ss << m_value;
    TextInput *text = input();
    if(text) {
        text->setText(ss.str());
    }
}
/*!
    Returns the minimum allowed value.
*/
float FloatInput::minimum() const {
    return m_minimum;
}
/*!
    Sets the \a minimum allowed value.
*/
void FloatInput::setMinimum(float minimum) {
    m_minimum = minimum;
}
/*!
    Returns the maximum allowed value.
*/
float FloatInput::maximum() const {
    return m_maximum;
}
/*!
    Sets the \a maximum allowed value.
*/
void FloatInput::setMaximum(float maximum) {
    m_maximum = maximum;
}
/*!
    Returns the single step value for incrementing or decrementing the FloatInput value.
*/
float FloatInput::singleStep() const {
    return m_singleStep;
}
/*!
    Sets the single \a step value for incrementing or decrementing the FloatInput value.
*/
void FloatInput::setSingleStep(float step) {
    m_singleStep = step;

    Button *increaseBtn = increaseButton();
    if(increaseBtn) {
        increaseBtn->actor()->setEnabled(m_singleStep != 0.0f);
    }

    Button *decreaseBtn = decreaseButton();
    if(decreaseBtn) {
        decreaseBtn->actor()->setEnabled(m_singleStep != 0.0f);
    }
}
/*!
    Returns the corners radiuses.
*/
Vector4 FloatInput::corners() const {
    return m_cornerRadius;
}
/*!
    Sets the \a corners radiuses.
*/
void FloatInput::setCorners(Vector4 corners) {
    m_cornerRadius = corners;

    Button *decreaseBtn = decreaseButton();
    if(decreaseBtn) {
        Frame *frame = decreaseBtn->background();
        if(frame) {
            Vector4 corners = m_cornerRadius;
            corners.y = corners.z = 0.0f;
            frame->setCorners(corners);
        }
    }

    Button *increaseBtn = increaseButton();
    if(increaseBtn) {
        Frame *frame = increaseBtn->background();
        if(frame) {
            Vector4 corners = m_cornerRadius;
            corners.x = corners.w = 0.0f;
            frame->setCorners(corners);
        }
    }
}
/*!
    Returns the increase value button.
*/
Button *FloatInput::increaseButton() const {
    return static_cast<Button *>(subWidget(gIncrease));
}
/*!
    Sets the increase value \a button.
*/
void FloatInput::setIncreaseButton(Button *button) {
    setSubWidget(gIncrease, button);
}
/*!
    Returns the decrease value button.
*/
Button *FloatInput::decreaseButton() const {
    return static_cast<Button *>(subWidget(gDecrease));
}
/*!
    Sets the decrease value \a button.
*/
void FloatInput::setDecreaseButton(Button *button) {
    setSubWidget(gDecrease, button);
}
/*!
    Returns the input field component.
*/
TextInput *FloatInput::input() const {
    return static_cast<TextInput *>(subWidget(gInput));
}
/*!
    Sets the \a input field component.
*/
void FloatInput::setInput(TextInput *input) {
    setSubWidget(gInput, input);
}
/*!
    Slot method called when the increase button is clicked. Increments the FloatInput value.
*/
void FloatInput::onIncrease() {
    setValue(m_value + m_singleStep);
}
/*!
    Slot method called when the decrease button is clicked. Decrements the FloatInput value.
*/
void FloatInput::onDecrease() {
    setValue(m_value - m_singleStep);
}
/*!
    Slot method called when editing of the input text is finished. Updates the FloatInput value based on the entered text.
*/
void FloatInput::onEditingFinished() {
    TextInput *current = input();
    if(current) {
        std::string text = current->text();
        if(!text.empty()) {
            setValue(stof(text));
        } else {
            setValue(value());
        }
    }
}
/*!
    \internal
    Internal method to compose the FloatInput component, creating and setting up the buttons and input text.
*/
void FloatInput::composeComponent() {
    Widget::composeComponent();

    const float width = 20.0f;

    Actor *text = Engine::composeActor(gTextInputClass, gTextInputClass, actor());

    TextInput *input = static_cast<TextInput *>(text->component(gTextInputClass));
    if(input) {
        connect(input, _SIGNAL(focusOut()), this, _SLOT(onEditingFinished()));
        connect(input, _SIGNAL(editingFinished()), this, _SLOT(onEditingFinished()));

        setValue(m_value);
        setInput(input);
    }
    RectTransform *textRect = static_cast<RectTransform *>(text->transform());
    textRect->setSize(Vector2(0.0f));
    textRect->setMargin(Vector4(0.0f, width, 0.0f, width));
    textRect->setAnchors(Vector2(0.0f), Vector2(1.0f));

    Sprite *arrow = Engine::loadResource<Sprite>(".embedded/ui.png");

    Actor *left = Engine::composeActor(gButtonClass, "Decrease", actor());
    Button *decreaseBtn = static_cast<Button *>(left->component(gButtonClass));
    if(decreaseBtn) {
        connect(decreaseBtn, _SIGNAL(clicked()), this, _SLOT(onDecrease()));

        decreaseBtn->setText("");
        decreaseBtn->setIconSize(Vector2(16.0f, 8.0f));
        setDecreaseButton(decreaseBtn);

        Image *icon = decreaseBtn->icon();
        if(icon) {
            icon->setSprite(arrow);
            icon->setItem("Arrow");
            RectTransform *rect = icon->rectTransform();
            if(rect) {
                rect->setRotation(Vector3(0.0f, 0.0f,-90.0f));
            }
        }
    }
    RectTransform *leftRect = static_cast<RectTransform *>(left->transform());
    leftRect->setSize(Vector2(width, 0.0f));
    leftRect->setAnchors(Vector2(0.0f), Vector2(0.0f, 1.0f));
    leftRect->setPivot(Vector2(0.0f));

    Actor *right = Engine::composeActor(gButtonClass, "Increase", actor());
    Button *increaseBtn = static_cast<Button *>(right->component(gButtonClass));
    if(increaseBtn) {
        connect(increaseBtn, _SIGNAL(clicked()), this, _SLOT(onIncrease()));

        increaseBtn->setText("");
        increaseBtn->setIconSize(Vector2(16.0f, 8.0f));
        setIncreaseButton(increaseBtn);

        Image *icon = increaseBtn->icon();
        if(icon) {
            icon->setSprite(arrow);
            icon->setItem("Arrow");
            RectTransform *rect = icon->rectTransform();
            if(rect) {
                rect->setRotation(Vector3(0.0f, 0.0f, 90.0f));
            }
        }
    }

    RectTransform *rightRect = static_cast<RectTransform *>(right->transform());
    rightRect->setSize(Vector2(width, 0.0f));
    rightRect->setAnchors(Vector2(1.0f, 0.0f), Vector2(1.0f));
    rightRect->setPivot(Vector2(1.0f, 1.0f));

    rectTransform()->setSize(Vector2(100.0f, 30.0f));

    setCorners(Vector4(4.0f));
}
