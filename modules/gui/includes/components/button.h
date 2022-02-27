#ifndef BUTTON_H
#define BUTTON_H

#include "abstractbutton.h"

class GUI_EXPORT Button : public AbstractButton {
    A_REGISTER(Button, AbstractButton, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()

private:
    void composeComponent() override;
};

#endif // BUTTON_H
