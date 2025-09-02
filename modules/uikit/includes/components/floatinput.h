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
