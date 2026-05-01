#ifndef BUTTON_H
#define BUTTON_H

#include "abstractbutton.h"

class UIKIT_EXPORT Button : public AbstractButton {
    A_OBJECT(Button, AbstractButton, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Button();

private:
    void setImage(Image *image) override;

    void composeComponent() override;
};

#endif // BUTTON_H
