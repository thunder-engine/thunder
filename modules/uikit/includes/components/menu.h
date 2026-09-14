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
#ifndef MENU_H
#define MENU_H

#include "frame.h"

class Font;

class UIKIT_EXPORT Menu : public Frame {
    A_OBJECT(Menu, Frame, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Font *, font, Menu::font, Menu::setFont, "editor=Asset")
    )
    A_METHODS(
        A_SIGNAL(Menu::aboutToHide),
        A_SIGNAL(Menu::aboutToShow),
        A_SIGNAL(Menu::triggered)
    )
    A_NOENUMS()

    struct MenuItem {
        enum Type { Action, Separator, Submenu };

        Type type = Action;
        TString text;

        Sprite *icon = nullptr;

        Menu *submenu = nullptr;

        bool enabled = true;
        bool hovered = false;
    };

public:
    Menu();
    ~Menu();

    void addAction(const TString &text, Sprite *icon = nullptr);
    void addSeparator();
    void addSubmenu(const TString &text, Menu *submenu, Sprite *icon = nullptr);

    void show(const Vector2 &position);
    void hide();

    TString itemText(int index);
    void setItemText(int index, const TString &text);

    Sprite *itemIcon(int index);
    void setItemIcon(int index, Sprite *icon);

    Font *font() const;
    void setFont(Font *font);

public: // signals
    void aboutToShow();
    void aboutToHide();
    void triggered(int);

private:
    void draw() override;
    void update(const Vector2 &pos) override;

    void updateSize();

    void composeComponent() override;

    static void fontUpdated(int state, void *ptr);

private:
    std::vector<MenuItem> m_items;

    Vector4 m_textColor = Vector4(1);
    Vector4 m_selectionColor = Vector4(0.01f, 0.6f, 0.89f, 1.0f);
    Vector4 m_separatorColor = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
    Vector2 m_iconSize = Vector2(16, 16);

    Font *m_font = nullptr;

    Mesh *m_textMesh;
    MaterialInstance *m_textMaterial;
    MaterialInstance *m_selectionMaterial;

    int m_fontSize = 14;

    int m_hoveredIndex = -1;
    int m_selectedIndex = -1;

    float m_separatorHeight = 5.0f;
    float m_rowHeight = 20.0f;
    float m_minWidth = 100.0f;

    bool m_dirtyText;
};

#endif // MENU_H
