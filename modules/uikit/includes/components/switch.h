#ifndef SWITCH_H
#define SWITCH_H

#include "checkbox.h"

class UIKIT_EXPORT Switch : public CheckBox {
    A_OBJECT(Switch, AbstractButton, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Switch();

protected:
    void update(const Vector2 &pos) override;

    void checkStateSet() override;

    void composeComponent() override;

    void boundChanged(const Vector2 &size) override;

private:
    float m_switchDuration;
    float m_currentFade;

};

#endif // SWITCH_H
