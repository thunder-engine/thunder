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
#include "components/menu.h"

#include "components/recttransform.h"
#include "components/canvas.h"

#include "resources/font.h"
#include "resources/material.h"

#include <input.h>

namespace  {
    const char *gTexture("mainTexture");
    const char *gUseSDF("useSdf");

    const char *gBackgroundColor("backgroundColor");
    const char *gBorderWidth("borderWidth");
    const char *gBorderRadius("borderRadius");
    const char *gBorderColor("borderColor");

    const char *gDefaultFrame(".embedded/Frame.shader");
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
        m_textMesh(Engine::objectCreate<Mesh>()),
        m_textMaterial(nullptr),
        m_selectionMaterial(nullptr),
        m_dirtyText(true) {

    Material *material = Engine::loadResource<Material>(".embedded/DefaultFont.shader");
    if(material) {
        m_textMaterial = material->createInstance();
        int sdf = 0;
        m_textMaterial->setInteger(gUseSDF, &sdf);
        material->subscribe(&Menu::materialUpdated, this);
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

    setEnabled(false);
}

Menu::~Menu() {
    if(m_font) {
        m_font->unsubscribe(this);
    }
    if(m_textMaterial) {
        m_textMaterial->material()->unsubscribe(this);
    }
    if(m_selectionMaterial) {
        m_selectionMaterial->material()->unsubscribe(this);
    }

    delete m_textMesh;
    delete m_textMaterial;
    delete m_selectionMaterial;
}

/*!
    Adds a section to the menu with the specified \a text and optional \a icon.
    Creates a clickable menu item that triggers an action when selected.
*/
void Menu::addAction(const TString &text, Sprite *icon) {
    Actor *actionActor = Engine::composeActor<Action>(text, actor());
    Action *action = actionActor ? actionActor->getComponent<Action>() : nullptr;
    if(!action) {
        return;
    }
    action->setText(text);
    addAction(action, icon);
}

Action *Menu::addAction(Action *action, Sprite *icon) {
    if(!action) {
        return nullptr;
    }
    if(action->actor() && action->actor()->parent() != actor()) {
        action->actor()->setParent(actor());
    }
    setSubWidget(action);

    MenuItem item;
    item.type = MenuItem::Action;
    item.action = action;
    item.text = action->text();
    item.icon = icon;
    item.enabled = action->isEnabled();
    item.checkable = action->isCheckable();
    item.checked = action->isChecked();
    item.shortcut = action->shortcutText();
    m_items.push_back(item);

    connect(action, _SIGNAL(changed()), this, _SLOT(onActionChanged()));
    m_dirtyText = true;
    updateSize();
    return action;
}
void Menu::setItemState(int index, bool enabled, bool checkable, bool checked) {
    if(index < 0 || index >= static_cast<int>(m_items.size()) ||
        m_items[index].type != MenuItem::Action) {
        return;
    }
    MenuItem &item = m_items[index];
    if(item.enabled != enabled || item.checkable != checkable || item.checked != checked) {
        item.action->setEnabled(enabled);
        item.action->setCheckable(checkable);
        item.action->setChecked(checked);
        item.enabled = item.action->isEnabled();
        item.checkable = item.action->isCheckable();
        item.checked = item.action->isChecked();
        m_dirtyText = true;
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
void Menu::addSubmenu(const TString &text, Menu *submenu, Sprite *icon) {
    MenuItem item;
    item.type = MenuItem::Submenu;
    item.text = text;
    item.submenu = submenu;
    item.icon = icon;
    m_items.push_back(item);
    if(submenu) {
        setSubWidget(submenu);
    }

    m_dirtyText = true;
    updateSize();
}
void Menu::setItemShortcut(int index, const TString &shortcut) {
    if(index < 0 || index >= static_cast<int>(m_items.size()) ||
        m_items[index].type != MenuItem::Action) {
        return;
    }
    if(m_items[index].shortcut != shortcut) {
        m_items[index].shortcut = shortcut;
        m_dirtyText = true;
        updateSize();
    }
}

void Menu::onActionChanged() {
    Action *action = dynamic_cast<Action *>(sender());
    if(!action) {
        return;
    }
    for(MenuItem &item : m_items) {
        if(item.action == action) {
            item.text = action->text();
            item.enabled = action->isEnabled();
            item.checkable = action->isCheckable();
            item.checked = action->isChecked();
            item.shortcut = action->shortcutText();
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
    aboutToShow();
    setEnabled(true);
    rectTransform()->setPosition(Vector3(position, 0.0f));
    repaint();
}
/*!
    Hides the menu.
*/
void Menu::hide() {
    if(isEnabled() && !m_hiding) {
        m_hiding = true;
        aboutToHide();
        m_hiding = false;
    }
    if(m_openSubmenu) {
        m_openSubmenu->hide();
        m_openSubmenu = nullptr;
    }
    setEnabled(false);
    repaint();
}
/*!
    Returns the text of the item at the specified \a index.
*/
TString Menu::itemText(int index) {
    if(index > -1 && index < m_items.size()) {
        return m_items[index].text;
    }
    return TString();
}
/*!
    Updates the \a text of an existing menu item at \a index of menu position.
*/
void Menu::setItemText(int index, const TString &text) {
    if(index > -1 && index < m_items.size()) {
        m_items[index].text = text;
        if(m_items[index].action) {
            m_items[index].action->setText(text);
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
        return m_items[index].icon;
    }
    return nullptr;
}
/*!
    Updates the \a icon of an existing menu item at \a index of menu position.
*/
void Menu::setItemIcon(int index, Sprite *icon) {
    if(index > -1 && index < m_items.size()) {
        m_items[index].icon = icon;
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
/*!
    \internal
    Internal method called to draw menu.
*/
void Menu::draw() {
    Frame::draw();

    RectTransform *rect = rectTransform();
    Vector2 size(rect->size());
    Matrix4 transform(rect->worldTransform());

    if(m_dirtyText && m_font && m_textMaterial) {
        m_textMesh->clear();
        Font::Settings settings = {m_fontSize, Alignment::Left | Alignment::Middle, Font::Additive, m_textColor};
        settings.boundaries = Vector2(size.x - 40.0f, m_rowHeight);
        settings.offset.x = 20.0f;
        settings.offset.y = -settings.boundaries.y - m_rowHeight;
        for(size_t i = 0; i < m_items.size(); ++i) {
            if(m_items[i].type != MenuItem::Separator) {
                TString label;
                if(m_items[i].checkable) {
                    label = m_items[i].checked ? "[x] " : "[ ] ";
                }
                label += m_items[i].text;
                if(m_items[i].type == MenuItem::Submenu) {
                    label += " >";
                } else if(!m_items[i].shortcut.isEmpty()) {
                    label += TString("    ") + m_items[i].shortcut;
                }
                settings.color = m_items[i].enabled ? m_textColor : m_textColor * 0.45f;
                m_font->composeMesh(m_textMesh, label, settings);
                settings.offset.y += m_rowHeight;
            } else {
                settings.offset.y -= m_separatorHeight;
            }
        }
        m_textMaterial->setTexture(gTexture, m_font->page());
        m_dirtyText = false;
    }

    Canvas *canvas = Menu::canvas();

    uint32_t hash = rect->hash();
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
}
/*!
    \internal
    Updates the menu. Handles input and triggers actions based on user interactions.
*/
void Menu::update(const Vector2 &pos) {
    Widget::update(pos);

    RectTransform *rect = rectTransform();
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
        RectTransform *submenuRect = hoveredSubmenu->rectTransform();
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

    if(Input::isMouseButtonDown(0)) {
        if(newHoveredIndex > -1 && m_items[newHoveredIndex].type == MenuItem::Action &&
            m_items[newHoveredIndex].enabled) {
            auto &item = m_items[m_hoveredIndex];
            if(item.action) {
                item.action->trigger();
            }
            triggered(newHoveredIndex);
        }

        if(!containsMenuAt(pos)) {
            hide();
        }
    }
}
/*!
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
    float height = 0.0f;
    float width = m_minWidth;

    for(auto &item : m_items) {
        if(item.type == MenuItem::Separator) {
            height += m_separatorHeight;
        } else {
            if(m_font && !item.text.isEmpty()) {
                TString label;
                if(item.checkable) {
                    label = "[ ] ";
                }
                label += item.text;
                if(item.type == MenuItem::Submenu) {
                    label += " >";
                }
                if(!item.shortcut.isEmpty()) {
                    label += TString("    ") + item.shortcut;
                }
                float textWidth = m_font->textWidth(label, m_fontSize, 0);
                width = std::max(width, textWidth + 40.0f);
            }
            height += m_rowHeight;
        }
    }

    rectTransform()->setSize(Vector2(width, height));
    repaint();
}
/*!
    \internal
*/
void Menu::composeComponent() {
    Frame::composeComponent();

    setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
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
    }

    menu->m_dirtyText = true;
    menu->repaint();
}
