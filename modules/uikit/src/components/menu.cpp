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
#include "menu.h"
#include "button.h"

#include "canvas.h"

#include "resources/font.h"
#include "resources/material.h"
#include "resources/mesh.h"

#include <algorithm>
#include <cstring>

Menu *Menu::m_currentMenu = nullptr;

namespace  {
    const char *gTexture("mainTexture");
    const char *gUseSDF("useSdf");
    const char *gColor("mainColor");

    const char *gBackgroundColor("backgroundColor");
    const char *gBorderWidth("borderWidth");
    const char *gBorderRadius("borderRadius");
    const char *gBorderColor("borderColor");

    const char *gDefaultFrame(".embedded/Frame.shader");

}

bool Menu::hasActionIcons() const {
    for(const MenuItem &item : m_items) {
        if(item.type == MenuItem::Action && item.action && item.action->icon()) {
            return true;
        }
    }
    return false;
}

/*!
    \class Menu
    \brief The Menu class represents a graphical user interface menu with options and actions.
    \inmodule Gui

    The Menu class represents a graphical user interface (GUI) menu that contains a list of options or actions that users can select.
    It provides a way to organize and display commands or choices in a structured manner, typically within a dropdown or context menu format.
    Menus are essential in GUI design for providing users with accessible options and actions within an application.
*/

Menu::Menu() :
        m_items(),
        m_textColor(Vector4(1)),
        m_selectionColor(Vector4(0.01f, 0.6f, 0.89f, 1.0f)),
        m_separatorColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f)),
        m_iconSize(Vector2(16, 16)),
        m_font(nullptr),
        m_checkSprite(nullptr),
        m_arrowSprite(nullptr),
        m_textMesh(Engine::objectCreate<Mesh>()),
        m_textMaterial(nullptr),
        m_checkMaterial(nullptr),
        m_arrowMaterial(nullptr),
        m_itemIconMaterial(nullptr),
        m_selectionMaterial(nullptr),
        m_checkInstanceBuffer(),
        m_arrowInstanceBuffer(),
        m_fontSize(14),
        m_hoveredIndex(-1),
        m_selectedIndex(-1),
        m_openSubmenu(nullptr),
        m_iconWorldHash(0),
        m_separatorHeight(5.0f),
        m_rowHeight(20.0f),
        m_minWidth(100.0f),
        m_hiding(false),
        m_dirtyText(true),
        m_dirtyCheckIcons(true),
        m_dirtyArrowIcons(true),
        m_visible(false) {

    Material *material = Engine::loadResource<Material>(".embedded/DefaultFont.shader");
    if(material) {
        m_textMaterial = material->createInstance();
        int sdf = 0;
        m_textMaterial->setInteger(gUseSDF, &sdf);
        material->subscribe(&Menu::materialUpdated, this);
    }

    Material *spriteMaterial = Engine::loadResource<Material>(".embedded/DefaultUI.shader");
    if(spriteMaterial) {
        m_checkMaterial = spriteMaterial->createInstance();
        m_arrowMaterial = spriteMaterial->createInstance();
        m_itemIconMaterial = spriteMaterial;
        spriteMaterial->subscribe(&Menu::materialUpdated, this);
    }

    Material *frameMaterial = Engine::loadResource<Material>(gDefaultFrame);
    if(frameMaterial) {
        m_selectionMaterial = frameMaterial->createInstance();

        Vector4 width(0.0f);
        m_selectionMaterial->setVector4(gBorderWidth, &width);
        m_selectionMaterial->setVector4(gBorderRadius, &m_borderRadius);
        m_selectionMaterial->setVector4(gBorderColor, &m_borderColor);
        m_selectionMaterial->setVector4(gBackgroundColor, &m_selectionColor);
        frameMaterial->subscribe(&Menu::materialUpdated, this);
    }

    setBackgroundColor(Vector4(0.14f, 0.14f, 0.14f, 1.0f));

}

Menu::~Menu() {
    if(Canvas *canvas = Menu::canvas()) {
        canvas->unregisterShortcuts(this);
        canvas->m_popupMenus.erase(std::remove(canvas->m_popupMenus.begin(), canvas->m_popupMenus.end(), this),
                                   canvas->m_popupMenus.end());
    }

    if(m_font) {
        m_font->unsubscribe(this);
    }
    if(m_checkSprite) {
        m_checkSprite->unsubscribe(this);
    }
    if(m_arrowSprite) {
        m_arrowSprite->unsubscribe(this);
    }
    if(m_textMaterial) {
        m_textMaterial->material()->unsubscribe(this);
    }
    if(m_checkMaterial) {
        m_checkMaterial->material()->unsubscribe(this);
    }
    if(m_arrowMaterial) {
        m_arrowMaterial->material()->unsubscribe(this);
    }
    if(m_itemIconMaterial) {
        m_itemIconMaterial->unsubscribe(this);
    }
    if(m_selectionMaterial) {
        m_selectionMaterial->material()->unsubscribe(this);
    }

    delete m_textMesh;
    delete m_textMaterial;
    delete m_checkMaterial;
    delete m_arrowMaterial;
    delete m_selectionMaterial;
    for(MenuItem &item : m_items) {
        delete item.iconMaterial;
    }
}

/*!
    Adds a section to the menu with the specified \a text and optional \a icon.
    Creates a clickable menu item that triggers an action when selected.
*/
Action *Menu::addAction(const TString &text, Sprite *icon, const TString &shortcut) {
    Action *action = Engine::objectCreate<Action>(text, this);
    if(!action) {
        return nullptr;
    }
    action->setText(text);
    if(icon) {
        action->setIcon(icon);
    }
    action->setShortcutText(shortcut);
    return addAction(action);
}

Action *Menu::addAction(Action *action) {
    if(!action) {
        return nullptr;
    }
    if(action->parent() != this) {
        action->setParent(this);
    }

    MenuItem item;
    item.type = MenuItem::Action;
    item.action = action;
    m_items.push_back(item);

    connect(action, _SIGNAL(changed()), this, _SLOT(onActionChanged()));
    m_dirtyText = true;
    updateSize();
    if(Canvas *canvas = Menu::canvas()) {
        canvas->registerShortcut(this, action);
    }
    return action;
}
void Menu::setItemState(int index, bool enabled, bool checkable, bool checked) {
    if(index < 0 || index >= static_cast<int>(m_items.size()) ||
        m_items[index].type != MenuItem::Action) {
        return;
    }
    MenuItem &item = m_items[index];
    if(item.action->isEnabled() != enabled || item.action->isCheckable() != checkable ||
            item.action->isChecked() != checked) {
        item.action->setEnabled(enabled);
        item.action->setCheckable(checkable);
        item.action->setChecked(checked);
        m_dirtyText = true;
        invalidateIconDraws();
        updateSize();
        repaint();
    }
}
/*!
    Adds a separator line to the menu.
    Creates a visual separator to group related menu items.
*/
void Menu::addSeparator() {
    MenuItem item;
    item.type = MenuItem::Separator;
    m_items.push_back(item);

    m_dirtyText = true;
    updateSize();
}
/*!
    Adds a \a submenu item with specified \a text and optional \a icon.
    Creates a menu item that opens a submenu when hovered or clicked.
*/
void Menu::addSubmenu(const TString &text, Menu *submenu) {
    MenuItem item;
    item.type = MenuItem::Submenu;
    item.label = text;
    item.submenu = submenu;
    m_items.push_back(item);
    if(submenu) {
        setSubWidget(submenu);
    }

    m_dirtyText = true;
    updateSize();
}

void Menu::onActionChanged() {
    Action *action = dynamic_cast<Action *>(sender());
    if(!action) {
        return;
    }
    for(MenuItem &item : m_items) {
        if(item.action == action) {
            if(item.iconMaterial) {
                item.iconMaterial->setTexture(gTexture,
                                              item.action->icon() ? item.action->icon()->texture() : nullptr);
            }
            invalidateIconDraws();
            if(Canvas *canvas = Menu::canvas()) {
                canvas->registerShortcut(this, action);
            }
            updateSize();
            m_dirtyText = true;
            repaint();
        }
    }
}
/*!
    Displays the menu at the specified \a position.
*/
void Menu::show(const Vector2 &position) {
    if(!m_visible) {
        m_visible = true;
        aboutToShow();
    }
    Menu *rootMenu = this;
    for(Widget *parent = parentWidget(); parent; parent = parent->parentWidget()) {
        if(Menu *parentMenu = dynamic_cast<Menu *>(parent)) {
            rootMenu = parentMenu;
        }
    }
    if(m_currentMenu && m_currentMenu != rootMenu) {
        m_currentMenu->hide();
    }
    m_currentMenu = rootMenu;
    if(Canvas *canvas = Menu::canvas()) {
        if(std::find(canvas->m_popupMenus.begin(), canvas->m_popupMenus.end(), this) == canvas->m_popupMenus.end()) {
            canvas->m_popupMenus.push_back(this);
        }
    }

    setEnabled(true);
    if(dynamic_cast<Button *>(parentWidget())) {
        setAnchors(Vector2(0.0f), Vector2(0.0f));
        setPivot(Vector2(0.0f, 1.0f));
    }
    setPosition(Vector3(position, 0.0f));

    Canvas *canvas = Menu::canvas();
    Widget *parent = parentWidget();
    if(canvas && parent) {
        const Vector4 canvasRegion = canvas->clipRegion();
        if(canvasRegion.z <= 0.0f || canvasRegion.w <= 0.0f) {
            raise();
            repaint();
            return;
        }
        Vector4 menuRegion = clipRegion();

        if(dynamic_cast<Menu *>(parent) && menuRegion.x + menuRegion.z > canvasRegion.x + canvasRegion.z &&
           pivot().x == 0.0f) {
            setPivot(Vector2(1.0f, pivot().y));
            menuRegion = clipRegion();
        } else if(dynamic_cast<Button *>(parent) && menuRegion.y < canvasRegion.y) {
            Vector2 currentPivot = pivot();
            Vector3 currentPosition = Menu::position();
            setPivot(Vector2(currentPivot.x, 0.0f));
            setPosition(Vector3(currentPosition.x, currentPosition.y + parent->size().y, currentPosition.z));
            menuRegion = clipRegion();
        }

        float offsetX = 0.0f;
        float offsetY = 0.0f;
        if(menuRegion.x < canvasRegion.x) {
            offsetX = canvasRegion.x - menuRegion.x;
        } else if(menuRegion.x + menuRegion.z > canvasRegion.x + canvasRegion.z) {
            offsetX = canvasRegion.x + canvasRegion.z - menuRegion.x - menuRegion.z;
        }
        if(menuRegion.y < canvasRegion.y) {
            offsetY = canvasRegion.y - menuRegion.y;
        } else if(menuRegion.y + menuRegion.w > canvasRegion.y + canvasRegion.w) {
            offsetY = canvasRegion.y + canvasRegion.w - menuRegion.y - menuRegion.w;
        }

        if(offsetX != 0.0f || offsetY != 0.0f) {
            Matrix4 parentInverse = parent->worldTransform().inverse();
            Vector3 localOrigin = parentInverse * Vector3(0.0f, 0.0f, 0.0f);
            Vector3 localOffset = parentInverse * Vector3(offsetX, offsetY, 0.0f) - localOrigin;
            Vector3 currentPosition = Menu::position();
            setPosition(Vector3(currentPosition.x + localOffset.x,
                                currentPosition.y + localOffset.y,
                                currentPosition.z));
        }
    }
    raise();

    repaint();
}
/*!
    Hides the menu.
*/
void Menu::hide() {
    if(m_visible && !m_hiding) {
        m_hiding = true;
        aboutToHide();
        m_hiding = false;
        m_visible = false;
        m_hoveredIndex = -1;
    }

    if(m_currentMenu == this) {
        m_currentMenu = nullptr;
    }

    if(m_openSubmenu) {
        m_openSubmenu->hide();
        m_openSubmenu = nullptr;
    }
    if(Canvas *canvas = Menu::canvas()) {
        canvas->m_popupMenus.erase(std::remove(canvas->m_popupMenus.begin(), canvas->m_popupMenus.end(), this),
                                   canvas->m_popupMenus.end());
    }
    repaint();
}

bool Menu::isVisible() const {
    return m_visible;
}
/*!
    Returns the text of the item at the specified \a index.
*/
TString Menu::itemText(int index) {
    if(index > -1 && index < m_items.size()) {
        return m_items[index].action ? m_items[index].action->text() : m_items[index].label;
    }
    return TString();
}
/*!
    Updates the \a text of an existing menu item at \a index of menu position.
*/
void Menu::setItemText(int index, const TString &text) {
    if(index > -1 && index < m_items.size()) {
        if(m_items[index].action) {
            m_items[index].action->setText(text);
        } else {
            m_items[index].label = text;
            updateSize();
        }
        m_dirtyText = true;
        repaint();
    }
}
/*!
    Returns the icon of the item at the specified \a index.
*/
Sprite *Menu::itemIcon(int index) {
    if(index > -1 && index < m_items.size()) {
        return m_items[index].action ? m_items[index].action->icon() :
                (m_items[index].type == MenuItem::Submenu ? m_arrowSprite : nullptr);
    }
    return nullptr;
}
/*!
    Updates the \a icon of an existing menu item at \a index of menu position.
*/
void Menu::setItemIcon(int index, Sprite *icon) {
    if(index > -1 && index < m_items.size()) {
        MenuItem &item = m_items[index];
        if(item.action) {
            item.action->setIcon(icon);
        }
        repaint();
    }
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *Menu::font() const {
    return m_font;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void Menu::setFont(Font *font) {
    if(m_font != font) {
        if(m_font) {
            m_font->unsubscribe(this);
        }

        m_font = font;
        if(m_font) {
            m_font->subscribe(&Menu::fontUpdated, this);
        }

        m_dirtyText = true;
        repaint();
    }
}
/*!
    Signal emitted when the menu is about to be shown.
*/
void Menu::aboutToShow() {
    emitSignal(_SIGNAL(aboutToShow()));
}
/*!
    Signal emitted when the menu is about to be hidden.
*/
void Menu::aboutToHide() {
    emitSignal(_SIGNAL(aboutToHide()));
}
/*!
    Signal emitted when a menu with \a index item is triggered.
*/
void Menu::triggered(int index) {
    emitSignal(_SIGNAL(triggered(int)), index);
}

void Menu::actionTriggered(Action *action) {
    emitSignal(_SIGNAL(actionTriggered(Action*)), Variant::fromValue(action));
}

/*!
    \internal
    Internal method called to draw menu.
*/
void Menu::draw() {
    if(!m_visible) {
        return;
    }

    Canvas *canvas = Menu::canvas();
    if(canvas && !canvas->m_drawingPopups) {
        if(std::find(canvas->m_popupMenus.begin(), canvas->m_popupMenus.end(), this) == canvas->m_popupMenus.end()) {
            canvas->m_popupMenus.push_back(this);
        }
        return;
    }

    Frame::draw();

    Vector2 size(Menu::size());
    Matrix4 transform(worldTransform());

    if(m_dirtyText && m_font && m_textMaterial) {
        m_textMesh->clear();
        Font::Settings settings = {m_fontSize, Alignment::Left | Alignment::Middle, Font::Additive, m_textColor};
        const float iconColumn = hasActionIcons() ? 24.0f : 0.0f;
        settings.boundaries = Vector2(size.x - 48.0f - iconColumn, m_rowHeight);
        settings.offset.y = m_rowHeight - size.y;
        for(size_t i = 0; i < m_items.size(); ++i) {
            if(m_items[i].type != MenuItem::Separator) {
                const bool checkable = m_items[i].action && m_items[i].action->isCheckable();
                const float checkOffset = checkable && iconColumn == 0.0f ? 32.0f : 40.0f;
                settings.offset.x = (checkable ? checkOffset : 20.0f) + iconColumn;
                TString label;
                label += m_items[i].action ? m_items[i].action->text() : m_items[i].label;
                const TString shortcut = m_items[i].action ? m_items[i].action->shortcutText() : TString();
                if(m_items[i].type != MenuItem::Submenu && !shortcut.isEmpty()) {
                    label += TString("    ") + shortcut;
                }
                const bool enabled = !m_items[i].action || m_items[i].action->isEnabled();
                settings.color = enabled ? m_textColor : m_textColor * 0.45f;
                m_font->composeMesh(m_textMesh, label, settings);
                settings.offset.y += m_rowHeight;
            } else {
                settings.offset.y += m_separatorHeight;
            }
        }
        m_textMaterial->setTexture(gTexture, m_font->page());
        m_dirtyText = false;
    }

    uint32_t hash = Menu::hash();
    if(m_iconWorldHash != hash) {
        m_iconWorldHash = hash;
        invalidateIconDraws();
    }
    if(m_textMaterial) {
        m_textMaterial->setTransform(transform, 0, hash);
    }

    if(m_selectionMaterial && m_hoveredIndex > -1 && m_items[m_hoveredIndex].type != MenuItem::Separator) {
        Matrix4 s;
        s[0] = size.x;
        s[5] = m_rowHeight;
        s[12] = size.x * 0.5f;
        float rowHeight = 0.0f;
        for(int i = 0; i < m_hoveredIndex; ++i) {
            rowHeight += m_items[i].type == MenuItem::Separator ? m_separatorHeight : m_rowHeight;
        }
        s[13] = size.y - rowHeight - s[5] * 0.5f;
        Mathf::hashCombine(hash, s[13]);

        m_selectionMaterial->setTransform(transform * s, 0, hash);
        canvas->drawRect(m_selectionMaterial, nullptr);
    }

    if(m_textMaterial) {
        canvas->drawMesh(m_textMesh, m_textMaterial);
    }

    if(m_dirtyCheckIcons) {
        updateCheckIcons();
    }
    Mesh *checkMesh = m_checkSprite ? m_checkSprite->mesh() : nullptr;
    if(checkMesh && !m_checkInstanceBuffer.empty() && m_checkMaterial && m_checkSprite->texture()) {
        canvas->drawMesh(checkMesh, m_checkMaterial);
    }

    if(m_itemIconMaterial) {
        float rowTop = size.y;
        for(MenuItem &item : m_items) {
            const float rowHeight = item.type == MenuItem::Separator ? m_separatorHeight : m_rowHeight;
            Sprite *icon = item.action ? item.action->icon() :
                    (item.type == MenuItem::Submenu ? m_arrowSprite : nullptr);
            if(item.type == MenuItem::Action && icon && icon->texture()) {
                if(!item.iconMaterial) {
                    item.iconMaterial = m_itemIconMaterial->createInstance();
                }
                Mesh *iconMesh = icon->mesh();
                const float spriteWidth = iconMesh ? iconMesh->bound().extent.x * 2.0f : 0.0f;
                const float spriteHeight = iconMesh ? iconMesh->bound().extent.y * 2.0f : 0.0f;
                if(iconMesh && item.iconMaterial && spriteWidth > 0.0f && spriteHeight > 0.0f) {
                    item.iconMaterial->setTexture(gTexture, icon->texture());
                    Matrix4 iconTransform;
                    const float iconScale = 16.0f / MAX(spriteWidth, spriteHeight);
                    iconTransform[0] = iconScale;
                    iconTransform[5] = iconScale;
                    const bool checkable = item.action->isCheckable();
                    iconTransform[12] = (checkable ? 40.0f : 20.0f) + 8.0f;
                    iconTransform[13] = rowTop - rowHeight * 0.5f;
                    Mathf::hashCombine(hash, iconTransform[12]);
                    Mathf::hashCombine(hash, iconTransform[13]);
                    item.iconMaterial->setTransform(transform * iconTransform, 0, hash);
                    canvas->drawMesh(iconMesh, item.iconMaterial);
                }
            }
            rowTop -= rowHeight;
        }
    }

    if(m_dirtyArrowIcons) {
        updateArrowIcons();
    }
    Mesh *arrowMesh = m_arrowSprite ? m_arrowSprite->mesh() : nullptr;
    if(arrowMesh && !m_arrowInstanceBuffer.empty() && m_arrowMaterial && m_arrowSprite->texture()) {
        canvas->drawMesh(arrowMesh, m_arrowMaterial);
    }
}
/*!
    \internal
    Updates the menu. Handles input and triggers actions based on user interactions.
*/
void Menu::update(const Vector2 &pos) {
    if(!m_visible) {
        return;
    }

    Widget::update(pos);

    Widget *rect = this;
    Vector2 localPos = rect->mapFromGlobal(pos.x, pos.y);

    int newHoveredIndex = -1;
    if(localPos.x >= 0 && localPos.x <= rect->size().x &&
        localPos.y >= 0 && localPos.y <= rect->size().y) {
        float rowTop = rect->size().y;
        for(size_t i = 0; i < m_items.size(); ++i) {
            const float height = m_items[i].type == MenuItem::Separator ? m_separatorHeight : m_rowHeight;
            if(localPos.y <= rowTop && localPos.y >= rowTop - height) {
                if(m_items[i].type != MenuItem::Separator) {
                    newHoveredIndex = static_cast<int>(i);
                }
                break;
            }
            rowTop -= height;
        }
    }

    if(newHoveredIndex != m_hoveredIndex) {
        m_hoveredIndex = newHoveredIndex;
        repaint();
    }

    Menu *hoveredSubmenu = nullptr;
    if(newHoveredIndex >= 0 && m_items[newHoveredIndex].type == MenuItem::Submenu) {
        hoveredSubmenu = m_items[newHoveredIndex].submenu;
    } else if(m_openSubmenu && m_openSubmenu->containsMenuAt(pos)) {
        hoveredSubmenu = m_openSubmenu;
    }
    if(m_openSubmenu && m_openSubmenu != hoveredSubmenu) {
        m_openSubmenu->hide();
        m_openSubmenu = nullptr;
    }
    if(hoveredSubmenu && m_openSubmenu != hoveredSubmenu) {
        Widget *submenuRect = hoveredSubmenu;
        submenuRect->setAnchors(Vector2(0.0f), Vector2(0.0f));
        submenuRect->setPivot(Vector2(0.0f, 0.0f));
        float rowTop = rect->size().y;
        for(int i = 0; i < newHoveredIndex; ++i) {
            rowTop -= m_items[i].type == MenuItem::Separator ? m_separatorHeight : m_rowHeight;
        }
        const float rowBottom = rowTop - m_rowHeight;
        hoveredSubmenu->show(Vector2(rect->size().x, rowBottom));
        m_openSubmenu = hoveredSubmenu;
    }
}

bool Menu::isHovered(const Vector2 &pos) {
    return m_visible && Widget::isHovered(pos);
}

bool Menu::onMouseDown(int x, int y) {
    if(!m_visible || x < 0 || x > size().x || y < 0 || y > size().y) {
        return false;
    }

    int itemIndex = -1;
    float rowBottom = 0.0f;
    for(size_t i = 0; i < m_items.size(); ++i) {
        const float rowHeight = m_items[i].type == MenuItem::Separator ? m_separatorHeight : m_rowHeight;
        if(y >= rowBottom && y < rowBottom + rowHeight) {
            if(m_items[i].type != MenuItem::Separator) {
                itemIndex = static_cast<int>(i);
            }
            break;
        }
        rowBottom += rowHeight;
    }

    if(itemIndex > -1 && m_items[itemIndex].type == MenuItem::Action &&
            m_items[itemIndex].action && m_items[itemIndex].action->isEnabled()) {
        MenuItem &item = m_items[itemIndex];
        if(item.action) {
            item.action->trigger();
        }
        triggered(itemIndex);
    }

    hide();
    Widget *parent = parentWidget();
    while(Menu *parentMenu = dynamic_cast<Menu *>(parent)) {
        parentMenu->hide();
        parent = parentMenu->parentWidget();
    }
    return true;
}
/**
    \internal
*/
bool Menu::containsMenuAt(const Vector2 &position) {
    if(isHovered(position)) {
        return true;
    }
    return m_openSubmenu && m_openSubmenu->containsMenuAt(position);
}
/*!
    \internal
*/
void Menu::updateSize() {
    invalidateIconDraws();

    float height = 0.0f;
    float width = m_minWidth;

    for(auto &item : m_items) {
        if(item.type == MenuItem::Separator) {
            height += m_separatorHeight;
        } else {
            const TString itemText = item.action ? item.action->text() : item.label;
            if(m_font && !itemText.isEmpty()) {
                TString label;
                label += itemText;
                const TString shortcut = item.action ? item.action->shortcutText() : TString();
                if(item.type != MenuItem::Submenu && !shortcut.isEmpty()) {
                    label += TString("    ") + shortcut;
                }
                float textWidth = m_font->textWidth(label, m_fontSize, 0);
                const bool checkable = item.action && item.action->isCheckable();
                const float checkColumnWidth = checkable && !hasActionIcons() ? 60.0f : 68.0f;
                width = std::max(width, textWidth + (checkable ? checkColumnWidth : 48.0f) +
                                 (hasActionIcons() ? 24.0f : 0.0f));
            }
            height += m_rowHeight;
        }
    }

    this->setSize(Vector2(width, height));
    repaint();
}

void Menu::invalidateIconDraws() {
    m_dirtyCheckIcons = true;
    m_dirtyArrowIcons = true;
}

void Menu::updateCheckIcons() {
    updateIconInstances(m_checkSprite, m_checkMaterial, m_checkInstanceBuffer, true);
    m_dirtyCheckIcons = false;
}

void Menu::updateArrowIcons() {
    updateIconInstances(m_arrowSprite, m_arrowMaterial, m_arrowInstanceBuffer, false);
    m_dirtyArrowIcons = false;
}

void Menu::updateIconInstances(Sprite *sprite, MaterialInstance *material, ByteArray &buffer, bool checkedIcons) {
    buffer.clear();
    if(!material) {
        return;
    }
    if(!sprite || !sprite->mesh() || !sprite->texture()) {
        material->setInstanceBuffer(nullptr);
        return;
    }

    const float pixelsPerUnit = sprite->pixelsPerUnit();
    if(pixelsPerUnit <= 0.0f) {
        material->setInstanceBuffer(nullptr);
        return;
    }

    const Vector4 bounds = sprite->bounds();
    const float spriteWidth = (bounds.z - bounds.x) / pixelsPerUnit;
    const float spriteHeight = (bounds.w - bounds.y) / pixelsPerUnit;
    if(spriteWidth <= 0.0f || spriteHeight <= 0.0f) {
        material->setInstanceBuffer(nullptr);
        return;
    }

    const Vector2 iconSize = checkedIcons ? Vector2(24.0f, 12.0f) : Vector2(16.0f);
    const float scale = checkedIcons ? 0.0f : iconSize.x / MAX(spriteWidth, spriteHeight);
    const uint32_t stride = material->instanceSize();
    const uint32_t uniformSize = material->material()->uniformSize();
    if(stride < sizeof(Matrix4) || uniformSize < sizeof(Matrix4)) {
        material->setInstanceBuffer(nullptr);
        return;
    }

    float rowTop = size().y;
    for(const MenuItem &item : m_items) {
        const float rowHeight = item.type == MenuItem::Separator ? m_separatorHeight : m_rowHeight;
        const bool hasIcon = checkedIcons ?
                (item.type != MenuItem::Separator && item.action && item.action->isCheckable() &&
                 item.action->isChecked()) :
                item.type == MenuItem::Submenu;
        if(hasIcon) {
            Matrix4 iconTransform;
            if(checkedIcons) {
                iconTransform[0] = iconSize.x / spriteWidth;
                iconTransform[5] = iconSize.y / spriteHeight;
                iconTransform[12] = 12.0f;
            } else {
                iconTransform[0] = 0.0f;
                iconTransform[1] = scale;
                iconTransform[4] = -scale;
                iconTransform[5] = 0.0f;
                iconTransform[12] = size().x - 12.0f;
            }
            iconTransform[13] = rowTop - rowHeight * 0.5f;

            const Vector4 color = !item.action || item.action->isEnabled() ?
                    m_textColor : m_textColor * 0.45f;
            material->setVector4(gColor, &color);
            const ByteArray &uniform = material->rawUniformBuffer();
            if(uniform.size() < uniformSize) {
                material->setInstanceBuffer(nullptr);
                buffer.clear();
                return;
            }

            const size_t offset = buffer.size();
            buffer.resize(offset + stride);
            std::memcpy(buffer.data() + offset, uniform.data(), uniformSize);
            Matrix4 worldTransform = this->worldTransform() * iconTransform;
            std::memcpy(buffer.data() + offset, &worldTransform, sizeof(Matrix4));
        }
        rowTop -= rowHeight;
    }

    material->setInstanceBuffer(nullptr);
    if(!buffer.empty()) {
        material->setInstanceBuffer(&buffer);
    }
}

void Menu::boundChanged(const Vector2 &size) {
    Frame::boundChanged(size);
    invalidateIconDraws();
}

/*!
    \internal
*/
void Menu::composeComponent() {
    Frame::composeComponent();

    setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
    m_checkSprite = Engine::loadResource<Sprite>(".embedded/ui.png/Check");
    if(m_checkSprite) {
        m_checkSprite->subscribe(&Menu::checkSpriteUpdated, this);
        if(m_checkMaterial) {
            m_checkMaterial->setTexture(gTexture, m_checkSprite->texture());
        }
    }
    m_arrowSprite = Engine::loadResource<Sprite>(".embedded/ui.png/Arrow");
    if(m_arrowSprite) {
        m_arrowSprite->subscribe(&Menu::arrowSpriteUpdated, this);
        if(m_arrowMaterial) {
            m_arrowMaterial->setTexture(gTexture, m_arrowSprite->texture());
        }
    }
}
/*!
    \internal
*/
void Menu::fontUpdated(int state, void *ptr) {
    if(state == Resource::Ready) {
        Menu *p = static_cast<Menu *>(ptr);
        p->m_dirtyText = true;
        p->repaint();
    }
}

void Menu::checkSpriteUpdated(int state, void *ptr) {
    Menu *menu = static_cast<Menu *>(ptr);
    if(!menu) {
        return;
    }

    if(state == Resource::Ready) {
        if(menu->m_checkMaterial) {
            menu->m_checkMaterial->setTexture(gTexture, menu->m_checkSprite->texture());
        }
        menu->m_dirtyCheckIcons = true;
    } else if(state == Resource::ToBeDeleted) {
        if(menu->m_checkSprite) {
            menu->m_checkSprite->unsubscribe(menu);
        }
        menu->m_checkSprite = nullptr;
        if(menu->m_checkMaterial) {
            menu->m_checkMaterial->setTexture(gTexture, nullptr);
        }
        menu->m_checkInstanceBuffer.clear();
        if(menu->m_checkMaterial) {
            menu->m_checkMaterial->setInstanceBuffer(nullptr);
        }
        menu->m_dirtyCheckIcons = false;
    }
    menu->repaint();
}

void Menu::arrowSpriteUpdated(int state, void *ptr) {
    Menu *menu = static_cast<Menu *>(ptr);
    if(!menu) {
        return;
    }

    if(state == Resource::Ready) {
        if(menu->m_arrowMaterial) {
            menu->m_arrowMaterial->setTexture(gTexture, menu->m_arrowSprite->texture());
        }
        menu->m_dirtyArrowIcons = true;
    } else if(state == Resource::ToBeDeleted) {
        if(menu->m_arrowSprite) {
            menu->m_arrowSprite->unsubscribe(menu);
        }
        menu->m_arrowSprite = nullptr;
        if(menu->m_arrowMaterial) {
            menu->m_arrowMaterial->setTexture(gTexture, nullptr);
        }
        menu->m_arrowInstanceBuffer.clear();
        if(menu->m_arrowMaterial) {
            menu->m_arrowMaterial->setInstanceBuffer(nullptr);
        }
        menu->m_dirtyArrowIcons = false;
    }
    menu->repaint();
}

void Menu::materialUpdated(int state, void *ptr) {
    Menu *menu = static_cast<Menu *>(ptr);
    if(!menu) {
        return;
    }

    if(state == Material::ToBeDeleted) {
        if(menu->m_textMaterial && menu->m_textMaterial->material()->state() == state) {
            delete menu->m_textMaterial;
            menu->m_textMaterial = nullptr;
        }
        if(menu->m_selectionMaterial && menu->m_selectionMaterial->material()->state() == state) {
            delete menu->m_selectionMaterial;
            menu->m_selectionMaterial = nullptr;
        }
        if(menu->m_checkMaterial && menu->m_checkMaterial->material()->state() == state) {
            delete menu->m_checkMaterial;
            menu->m_checkMaterial = nullptr;
        }
        if(menu->m_arrowMaterial && menu->m_arrowMaterial->material()->state() == state) {
            delete menu->m_arrowMaterial;
            menu->m_arrowMaterial = nullptr;
        }
        if(menu->m_itemIconMaterial && menu->m_itemIconMaterial->state() == state) {
            menu->m_itemIconMaterial = nullptr;
            for(MenuItem &item : menu->m_items) {
                delete item.iconMaterial;
                item.iconMaterial = nullptr;
            }
        }
    }

    menu->m_dirtyText = true;
    menu->invalidateIconDraws();
    menu->repaint();
}
