#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include "abstractbutton.h"

class Menu;

class UIKIT_EXPORT ToolButton : public AbstractButton {
    A_OBJECT(ToolButton, AbstractButton, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_SIGNAL(ToolButton::itemChanged),
        A_SLOT(ToolButton::showMenu),
        A_SLOT(ToolButton::hideMenu),
        A_SLOT(ToolButton::onTriggered)
    )

    ToolButton();

    Menu *menu() const;
    void setMenu(Menu *menu);

    void showMenu();
    void hideMenu();

    void itemChanged();

private:
    void composeComponent() override;

    void onTriggered(int index);

private:
    String m_currentItem;

};

#endif // TOOLBUTTON_H
