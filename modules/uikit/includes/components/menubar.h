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
#ifndef MENUBAR_H
#define MENUBAR_H

#include <widget.h>

#include <functional>
#include <cstdint>
#include <map>
#include <vector>

class NativeMenuBar;
class NativeMenu;
class Button;
class Canvas;
class Menu;
class Window;

class UIKIT_EXPORT MenuBar : public Widget {
    A_OBJECT(MenuBar, Widget, UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_SLOT(MenuBar::onMenuButtonClicked),
        A_SLOT(MenuBar::onMenuTriggered)
    )
    A_NOENUMS()

public:
    class UIKIT_EXPORT MenuModel {
    public:
        void addAction(const TString &text, const std::function<void()> &handler = {});
        size_t addCheckableAction(const TString &text, const std::function<void()> &handler = {},
                                  bool checked = false);
        void setActionChecked(size_t index, bool checked);
        MenuModel *addMenu(const TString &title);
        void addSeparator();

    private:
        friend class MenuBar;

        MenuModel(MenuBar *owner, NativeMenu *menu);

        MenuBar *m_owner;
        NativeMenu *m_menu;
    };

    MenuBar();
    ~MenuBar() override;

    MenuModel *addMenu(const TString &title);
    bool isUsingNativeMenuBar() const;
    // Attaches the widget to a Canvas and its native model to the window when supported.
    bool attachToCanvas(Canvas *canvas, Window *window);
    void detachFromWindow();

    void draw() override;
    void update(const Vector2 &position) override;

private:
    MenuModel *createMenuModel(NativeMenu *menu);

    Menu *buildMenu(NativeMenu *model, Widget *parent, const TString &name);
    void rebuild();
    void clearWidgets();
    void hideMenus();

    void onMenuButtonClicked();
    void onMenuTriggered(int index);

private:
    NativeMenuBar *m_menuBar;
    Window *m_window;
    std::vector<MenuModel *> m_menuModels;
    std::uint64_t m_changeSubscription;
    std::vector<Button *> m_buttons;
    std::vector<Menu *> m_topMenus;
    std::map<Menu *, std::vector<std::pair<NativeMenu *, size_t>>> m_actionMap;

    int m_activeMenuIndex;
    bool m_dirty;
};

#endif // MENUBAR_H
