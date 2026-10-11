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
#ifndef MENUBARWIDGET_H
#define MENUBARWIDGET_H

#include "widget.h"

#include <os/menubar.h>

#include <map>
#include <vector>

class NativeMenuBar;
class NativeMenu;
class Button;
class Canvas;
class Menu;

class UIKIT_EXPORT MenuBar : public Widget {
    A_OBJECT(MenuBar, Widget, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_SLOT(MenuBar::onMenuButtonClicked),
        A_SLOT(MenuBar::onMenuTriggered)
    )
    A_NOENUMS()

public:
    MenuBar();
    ~MenuBar() override;

    // The menu model must outlive this widget.
    void setMenuBar(NativeMenuBar *menuBar);
    NativeMenuBar *menuBar() const;
    // Attaches this fallback widget to a Canvas on platforms without native menus.
    bool attachToCanvas(Canvas *canvas);

    void draw() override;
    void update(const Vector2 &position) override;

private:
    Menu *buildMenu(NativeMenu *model, Actor *parent, const TString &name);
    void rebuild();
    void clearWidgets();
    void hideMenus();

    void onMenuButtonClicked();
    void onMenuTriggered(int index);

private:
    NativeMenuBar *m_menuBar;
    NativeMenuBar::ChangeSubscription m_changeSubscription;
    std::vector<Button *> m_buttons;
    std::vector<Menu *> m_topMenus;
    std::map<Menu *, std::vector<std::pair<NativeMenu *, size_t>>> m_actionMap;

    int m_activeMenuIndex;
    bool m_dirty;
};

#endif // MENUBARWIDGET_H
