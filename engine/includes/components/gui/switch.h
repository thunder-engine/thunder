#ifndef SWITCH_H
#define SWITCH_H

#include "abstractbutton.h"

class ENGINE_EXPORT Switch : public AbstractButton {
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

    Frame *knobGraphic() const;
    void setKnobGraphic(Frame *image);

    Vector4 knobColor() const;
    void setKnobColor(const Vector4 color);

private:
    void update() override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void setMirrored(bool flag) override;

    void checkStateSet() override;

    void composeComponent() override;

    void onReferenceDestroyed() override;

private:
    Vector4 m_knobColor;

    Frame *m_knobGraphic;

    float m_switchDuration;
    float m_currentFade;

};

#endif // SWITCH_H
