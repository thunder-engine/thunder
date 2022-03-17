#ifndef SWITCH_H
#define SWITCH_H

#include "abstractbutton.h"

class SwitchPrivate;

class GUI_EXPORT Switch : public AbstractButton {
    A_REGISTER(Switch, AbstractButton, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Image *, knobGraphic, Switch::knobGraphic, Switch::setKnobGraphic, "editor=Component"),
        A_PROPERTY(float, transitionDuration, Switch::switchDuration, Switch::setSwitchDuration),
        A_PROPERTYEX(Vector4, knobColor, Switch::knobColor, Switch::setKnobColor, "editor=Color")
    )
    A_NOMETHODS()

    Switch();
    ~Switch();

    float switchDuration() const;
    void setSwitchDuration(float duration);

    Image *knobGraphic() const;
    void setKnobGraphic(Image *image);

    Vector4 knobColor() const;
    void setKnobColor(const Vector4 color);

    bool isOn() const;

private:
    void update() override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void onClicked() override;

    void composeComponent() override;

    void onReferenceDestroyed();

private:
    SwitchPrivate *p_ptr;

};

#endif // SWITCH_H
