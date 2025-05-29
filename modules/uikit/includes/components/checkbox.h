#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "abstractbutton.h"

class UIKIT_EXPORT CheckBox : public AbstractButton {
    A_OBJECT(CheckBox, AbstractButton, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Image *, knobGraphic, CheckBox::knobGraphic, CheckBox::setKnobGraphic, "editor=Component"),
        A_PROPERTYEX(Vector4, knobColor, CheckBox::knobColor, CheckBox::setKnobColor, "editor=Color")
    )
    A_NOMETHODS()

public:
    CheckBox();

    Image *knobGraphic() const;
    void setKnobGraphic(Image *knob);

    Vector4 knobColor() const;
    void setKnobColor(const Vector4 color);

    void setMirrored(bool flag) override;

private:
    void checkStateSet() override;

    void composeComponent() override;

private:
    Vector4 m_knobColor;

};

#endif // CHECKBOX_H
