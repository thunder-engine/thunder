/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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
