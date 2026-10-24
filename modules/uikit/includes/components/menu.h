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

#include <font.h>
#include <frame.h>
#include <action.h>

class Mesh;
class Material;

class UIKIT_EXPORT Menu : public Frame {
    A_OBJECT(Menu, Frame, UI)

    A_PROPERTIES(
        A_PROPERTYEX(Font *, font, Menu::font, Menu::setFont, "editor=Asset")
    )
    A_METHODS(
        A_SLOT(Menu::onActionChanged),
        A_SIGNAL(Menu::aboutToHide),
        A_SIGNAL(Menu::aboutToShow),
        A_SIGNAL(Menu::triggered),
        A_SIGNAL(Menu::actionTriggered)
    )
    A_NOENUMS()

    struct MenuItem {
        enum Type { Action, Separator, Submenu };

        MenuItem() :
                type(Action),
                label(),
                action(nullptr),
                submenu(nullptr),
                iconMaterial(nullptr) {
        }

        Type type;
        TString label;

        ::Action *action;
        Menu *submenu;
        MaterialInstance *iconMaterial;
    };

public:
    Menu();
    ~Menu();

    Action *addAction(const TString &text, Sprite *icon = nullptr, const TString &shortcut = TString());
    Action *addAction(Action *action);
    void addSeparator();
    void addSubmenu(const TString &text, Menu *submenu);
    void setItemState(int index, bool enabled, bool checkable, bool checked);

    void show(const Vector2 &position);
    void hide();
    bool isVisible() const;

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
    void actionTriggered(Action *action);

private:
    friend class Canvas;

    void draw() override;
    void update(const Vector2 &pos) override;
    bool isHovered(const Vector2 &pos) override;
    bool onMouseDown(int x, int y) override;

    void updateSize();
    void updateCheckIcons();
    void updateArrowIcons();
    void updateIconInstances(Sprite *sprite, MaterialInstance *material, ByteArray &buffer, bool checkedIcons);
    void invalidateIconDraws();
    bool hasActionIcons() const;
    bool containsMenuAt(const Vector2 &position);
    void onActionChanged();

    void composeComponent() override;
    void boundChanged(const Vector2 &size) override;

    static void fontUpdated(int state, void *ptr);
    static void checkSpriteUpdated(int state, void *ptr);
    static void arrowSpriteUpdated(int state, void *ptr);
    static void materialUpdated(int state, void *ptr);

private:
    std::vector<MenuItem> m_items;

    Vector4 m_textColor;
    Vector4 m_selectionColor;
    Vector4 m_separatorColor;
    Vector2 m_iconSize;

    Font *m_font;
    Sprite *m_checkSprite;
    Sprite *m_arrowSprite;

    static Menu *m_currentMenu;

    Mesh *m_textMesh;
    MaterialInstance *m_textMaterial;
    MaterialInstance *m_checkMaterial;
    MaterialInstance *m_arrowMaterial;
    Material *m_itemIconMaterial;
    MaterialInstance *m_selectionMaterial;
    ByteArray m_checkInstanceBuffer;
    ByteArray m_arrowInstanceBuffer;

    int m_fontSize;

    int m_hoveredIndex;
    int m_selectedIndex;
    Menu *m_openSubmenu;
    uint32_t m_iconWorldHash;

    float m_separatorHeight;
    float m_rowHeight;
    float m_minWidth;

    bool m_hiding;
    bool m_dirtyText;
    bool m_dirtyCheckIcons;
    bool m_dirtyArrowIcons;
    bool m_visible;
};

#endif // MENU_H
