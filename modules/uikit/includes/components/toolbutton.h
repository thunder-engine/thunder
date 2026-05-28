#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include "button.h"

class Menu;

class UIKIT_EXPORT ToolButton : public Button {
    A_OBJECT(ToolButton, Button, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_SLOT(ToolButton::showMenu),
        A_SLOT(ToolButton::hideMenu),
        A_SLOT(ToolButton::onTriggered)
    )
    A_NOENUMS()

    ToolButton();

    Menu *menu() const;
    void setMenu(Menu *menu);

    void showMenu();
    void hideMenu();

private:
    void composeComponent() override;

    void onTriggered(int index);

private:
    TString m_currentItem;

};

#endif // TOOLBUTTON_H
