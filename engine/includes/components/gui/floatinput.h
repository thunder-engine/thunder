#ifndef FLOATINPUT_H
#define FLOATINPUT_H

#include "widget.h"

class TextInput;
class Button;

class ENGINE_EXPORT FloatInput : public Widget {
    A_REGISTER(FloatInput, Widget, Components/UI)

    A_METHODS(
        A_SLOT(FloatInput::onIncrease),
        A_SLOT(FloatInput::onDecrease)
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

protected:
    void onIncrease();
    void onDecrease();

    void composeComponent() override;

private:
    Vector4 m_cornerRadius;

    Button *m_increaseBtn;
    Button *m_decreaseBtn;

    TextInput *m_input;

    float m_value;

    float m_singleStep;

    float m_minimum;
    float m_maximum;

};

#endif // FLOATINPUT_H
