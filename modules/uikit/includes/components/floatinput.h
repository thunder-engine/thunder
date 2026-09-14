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
#ifndef FLOATINPUT_H
#define FLOATINPUT_H

#include "widget.h"

class LineEdit;
class Button;

class UIKIT_EXPORT FloatInput : public Widget {
    A_OBJECT(FloatInput, Widget, Components/UI)

    A_PROPERTIES(
        A_PROPERTY(float, value, FloatInput::value, FloatInput::setValue),
        A_PROPERTY(float, minimum, FloatInput::minimum, FloatInput::setMinimum),
        A_PROPERTY(float, maximum, FloatInput::maximum, FloatInput::setMaximum),
        A_PROPERTY(float, singleStep, FloatInput::singleStep, FloatInput::setSingleStep),
        A_PROPERTY(Vector4, corners, FloatInput::corners, FloatInput::setCorners),
        A_PROPERTYEX(Button, increaseButton, FloatInput::increaseButton, FloatInput::setIncreaseButton, "editor=Component"),
        A_PROPERTYEX(Button, decreaseButton, FloatInput::decreaseButton, FloatInput::setDecreaseButton, "editor=Component"),
        A_PROPERTYEX(LineEdit, input, FloatInput::input, FloatInput::setInput, "editor=Component")
    )
    A_METHODS(
        A_SLOT(FloatInput::onIncrease),
        A_SLOT(FloatInput::onDecrease),
        A_SLOT(FloatInput::onEditingFinished)
    )
    A_NOENUMS()

public:
    FloatInput();

    float value() const;
    void setValue(float value);

    float minimum() const;
    void setMinimum(float minimum);

    float maximum() const;
    void setMaximum(float maximum);

    float singleStep() const;
    void setSingleStep(float);

    Vector4 corners() const;
    void setCorners(Vector4 corners);

    Button *increaseButton() const;
    void setIncreaseButton(Button *button);

    Button *decreaseButton() const;
    void setDecreaseButton(Button *button);

    LineEdit *input() const;
    void setInput(LineEdit *input);

protected: // slots
    void onIncrease();
    void onDecrease();

    void onEditingFinished();

    void composeComponent() override;

private:
    Vector4 m_cornerRadius;

    float m_value;

    float m_singleStep;

    float m_minimum;
    float m_maximum;

};

#endif // FLOATINPUT_H
