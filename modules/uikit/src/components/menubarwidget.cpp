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
#include "components/menubarwidget.h"

#include "components/button.h"
#include "components/action.h"
#include "components/canvas.h"
#include "components/menu.h"

#include <components/actor.h>
#include <engine.h>
#include <font.h>
#include <input.h>
#include <os/nativemenu.h>

#include <algorithm>
#include <iterator>

namespace {
    const float kMenuBarHeight = 28.0f;
    const float kMenuHorizontalPadding = 16.0f;

}

MenuBar::MenuBar() :
        m_menuBar(nullptr),
        m_changeSubscription(0),
        m_activeMenuIndex(-1),
        m_dirty(false) {
}

MenuBar::~MenuBar() {
    if(m_menuBar && m_changeSubscription) {
        m_menuBar->unsubscribeChanged(m_changeSubscription);
    }
}

void MenuBar::setMenuBar(NativeMenuBar *menuBar) {
    if(m_menuBar == menuBar) {
        return;
    }
    if(m_menuBar && m_changeSubscription) {
        m_menuBar->unsubscribeChanged(m_changeSubscription);
    }
    m_menuBar = menuBar;
    m_changeSubscription = 0;
    if(m_menuBar) {
        m_changeSubscription = m_menuBar->subscribeChanged([this]() {
            m_dirty = true;
            repaint();
        });
    }
    rebuild();
}

NativeMenuBar *MenuBar::menuBar() const {
    return m_menuBar;
}

bool MenuBar::attachToCanvas(Canvas *canvas) {
    if(!canvas) {
        return false;
    }
    setParent(canvas);
    setPivot(Vector2(0.0f, 1.0f));
    setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
    setSize(Vector2(0.0f, kMenuBarHeight));
    setPosition(Vector3(0.0f, 0.0f, 0.0f));

    return true;
}

void MenuBar::draw() {
    if(m_dirty) {
        rebuild();
    }
    Widget::draw();
}

void MenuBar::update(const Vector2 &position) {
    if(m_dirty) {
        rebuild();
    }
    Widget::update(position);
    if(!m_menuBar || m_menuBar->isAttached()) {
        return;
    }

    int modifiers = 0;
    if(Input::isKey(Input::KEY_LEFT_SHIFT) || Input::isKey(Input::KEY_RIGHT_SHIFT)) {
        modifiers |= Action::Shift;
    }
    if(Input::isKey(Input::KEY_LEFT_CONTROL) || Input::isKey(Input::KEY_RIGHT_CONTROL)) {
        modifiers |= Action::Control;
    }
    if(Input::isKey(Input::KEY_LEFT_ALT) || Input::isKey(Input::KEY_RIGHT_ALT)) {
        modifiers |= Action::Alt;
    }
    if(Input::isKey(Input::KEY_LEFT_SUPER) || Input::isKey(Input::KEY_RIGHT_SUPER)) {
        modifiers |= Action::Super;
    }
    for(int key = Input::KEY_SPACE; key <= Input::KEY_MENU; ++key) {
        if(Input::isKeyDown(static_cast<Input::KeyCode>(key)) && m_menuBar->triggerShortcut(key, modifiers)) {
            break;
        }
    }
}

Menu *MenuBar::buildMenu(NativeMenu *model, Widget *parent, const TString &name) {
    if(!model || !parent) {
        return nullptr;
    }

    Menu *view = Widget::createWidget<Menu>(name, parent);

    std::vector<std::pair<NativeMenu *, size_t>> &actions = m_actionMap[view];
    const auto &items = model->items();
    for(size_t itemIndex = 0; itemIndex < items.size(); ++itemIndex) {
        const NativeMenu::Item &item = items[itemIndex];
        if(item.separator) {
            view->addSeparator();
            actions.emplace_back(nullptr, itemIndex);
        } else if(item.submenu) {
            Menu *submenu = buildMenu(item.submenu, view, TString(item.submenu->title()));
            if(!submenu) {
                actions.emplace_back(nullptr, itemIndex);
                continue;
            }
            view->addSubmenu(item.text, submenu);
            actions.emplace_back(nullptr, itemIndex);
        } else {
            Action *action = Widget::createWidget<Action>(item.text, view);
            if(!action) {
                actions.emplace_back(nullptr, itemIndex);
                continue;
            }
            action->setText(item.text);
            action->setEnabled(item.enabled);
            action->setCheckable(item.checkable);
            action->setChecked(item.checked);
            action->setShortcut(item.shortcut);
            action->setShortcutModifiers(item.shortcutModifiers);
            view->addAction(action);
            actions.emplace_back(model, itemIndex);
        }
    }
    connect(view, _SIGNAL(triggered(int)), this, _SLOT(onMenuTriggered(int)));
    return view;
}

void MenuBar::rebuild() {
    m_dirty = false;
    clearWidgets();
    if(!m_menuBar) {
        return;
    }

    Font *font = Engine::loadResource<Font>(".embedded/Roboto.ttf");
    float offset = 0.0f;
    for(const auto &model : m_menuBar->menus()) {
        Button *button = Widget::createWidget<Button>(model->title(), this);
        if(!button) {
            continue;
        }

        button->setText(model->title());
        button->setFont(font);
        const float labelWidth = font ? font->textWidth(model->title(), button->fontSize(), 0) : 60.0f;
        const float width = labelWidth + kMenuHorizontalPadding * 2.0f;
        button->setPivot(Vector2(0.0f, 1.0f));
        button->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));
        button->setSize(Vector2(width, kMenuBarHeight));
        button->setPosition(Vector3(offset, 0.0f, 0.0f));
        offset += width;

        Menu *popup = buildMenu(model.get(), button, model->title() + TString("-popup"));
        if(popup) {
            button->setSubWidget(popup);
            m_buttons.push_back(button);
            m_topMenus.push_back(popup);
            connect(button, _SIGNAL(clicked()), this, _SLOT(onMenuButtonClicked()));
        }
    }
    repaint();
}

void MenuBar::clearWidgets() {
    hideMenus();
    m_actionMap.clear();
    m_buttons.clear();
    m_topMenus.clear();
    m_activeMenuIndex = -1;

    for(Object *child : getChildren()) {
        Button *childActor = dynamic_cast<Button *>(child);
        if(childActor) {
            childActor->setEnabled(false);
            childActor->deleteLater();
        }
    }
}

void MenuBar::hideMenus() {
    for(Menu *menu : m_topMenus) {
        if(menu) {
            menu->hide();
        }
    }
    m_activeMenuIndex = -1;
}

void MenuBar::onMenuButtonClicked() {
    Button *button = dynamic_cast<Button *>(sender());
    auto it = std::find(m_buttons.begin(), m_buttons.end(), button);
    if(it == m_buttons.end()) {
        return;
    }
    const int index = static_cast<int>(std::distance(m_buttons.begin(), it));
    if(index == m_activeMenuIndex && m_topMenus[index]->isEnabled()) {
        hideMenus();
        return;
    }

    hideMenus();
    m_activeMenuIndex = index;
    m_topMenus[index]->show(Vector2());
}

void MenuBar::onMenuTriggered(int index) {
    Menu *menu = dynamic_cast<Menu *>(sender());
    auto it = m_actionMap.find(menu);
    if(it == m_actionMap.end() || index < 0 || index >= static_cast<int>(it->second.size())) {
        return;
    }
    const auto &entry = it->second[index];
    if(entry.first) {
        entry.first->triggerAction(entry.second);
    }
    hideMenus();
}
