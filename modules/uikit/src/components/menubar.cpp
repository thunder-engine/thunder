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
#include "menubar.h"

#include "button.h"
#include "action.h"
#include "canvas.h"
#include "menu.h"

#include <engine.h>
#include <font.h>
#include <input.h>
#include <log.h>
#include <window.h>

#include <os/menubar.h>
#include <os/nativemenu.h>

#include <algorithm>
#include <iterator>

namespace {
    const float kMenuBarHeight = 22.0f;
    const float kMenuHorizontalPadding = 16.0f;
}

MenuBar::MenuBar() :
        m_menuBar(new NativeMenuBar),
        m_window(nullptr),
        m_changeSubscription(0),
        m_activeMenuIndex(-1),
        m_dirty(false) {

    m_changeSubscription = m_menuBar->subscribeChanged([this]() {
        m_dirty = true;
        repaint();
    });
}

MenuBar::~MenuBar() {
    m_menuBar->unsubscribeChanged(m_changeSubscription);
    detachFromWindow();
    for(MenuModel *model : m_menuModels) {
        delete model;
    }
    delete m_menuBar;
}

void MenuBar::detachFromWindow() {
    if(m_window) {
        m_window->setMenuBar(nullptr);
        m_window = nullptr;
    }
}

MenuBar::MenuModel::MenuModel(MenuBar *owner, NativeMenu *menu) :
        m_owner(owner),
        m_menu(menu) {
}

void MenuBar::MenuModel::addAction(const TString &text, const std::function<void()> &handler) {
    if(m_menu) {
        m_menu->addAction(text, handler);
    }
}

size_t MenuBar::MenuModel::addCheckableAction(const TString &text,
                                               const std::function<void()> &handler,
                                               bool checked) {
    if(!m_menu) {
        return 0;
    }
    const size_t index = m_menu->items().size();
    m_menu->addAction(text, handler, true, checked);
    return index;
}

void MenuBar::MenuModel::setActionChecked(size_t index, bool checked) {
    if(m_menu) {
        m_menu->setActionChecked(index, checked);
    }
}

MenuBar::MenuModel *MenuBar::MenuModel::addMenu(const TString &title) {
    if(!m_menu || !m_owner) {
        return nullptr;
    }
    return m_owner->createMenuModel(m_menu->addMenu(title));
}

void MenuBar::MenuModel::addSeparator() {
    if(m_menu) {
        m_menu->addSeparator();
    }
}

MenuBar::MenuModel *MenuBar::addMenu(const TString &title) {
    return createMenuModel(m_menuBar->addMenu(title));
}

MenuBar::MenuModel *MenuBar::createMenuModel(NativeMenu *menu) {
    if(!menu) {
        return nullptr;
    }
    MenuModel *model = new MenuModel(this, menu);
    m_menuModels.push_back(model);
    return model;
}

bool MenuBar::isUsingNativeMenuBar() const {
    return m_menuBar->isAttached();
}

bool MenuBar::attachToCanvas(Canvas *canvas, Window *window) {
    if(!canvas) {
        return false;
    }
    if(parentWidget() != canvas) {
        setParent(canvas);
    }

    Widget *rect = this;
    rect->setPivot(Vector2(0.0f, 1.0f));
    rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
    rect->setSize(Vector2(0.0f, kMenuBarHeight));
    rect->setPosition(Vector3(0.0f, 0.0f, 0.0f));

    m_window = window;
    if(m_window && !m_window->setMenuBar(m_menuBar)) {
        aWarning() << "Failed to attach the native menu bar; using the UiKit menu bar.";
    }

    return true;
}

void MenuBar::draw() {
    if(isUsingNativeMenuBar()) {
        return;
    }
    if(m_dirty) {
        rebuild();
    }
    Widget::draw();
}

void MenuBar::update(const Vector2 &position) {
    if(isUsingNativeMenuBar()) {
        return;
    }
    if(m_dirty) {
        rebuild();
    }
    Widget::update(position);
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

    Widget *menuActor = Widget::createWidget<Menu>(name, parent);
    Menu *view = menuActor ? dynamic_cast<Menu *>(menuActor) : nullptr;
    if(!view) {
        return nullptr;
    }

    std::vector<std::pair<NativeMenu *, size_t>> &actions = m_actionMap[view];
    const auto &items = model->items();
    for(size_t itemIndex = 0; itemIndex < items.size(); ++itemIndex) {
        const NativeMenu::Item &item = items[itemIndex];
        if(item.separator) {
            view->addSeparator();
            actions.emplace_back(nullptr, itemIndex);
        } else if(item.submenu) {
            Menu *submenu = buildMenu(item.submenu, menuActor, TString(item.submenu->title()));
            if(!submenu) {
                actions.emplace_back(nullptr, itemIndex);
                continue;
            }
            view->addSubmenu(item.text, submenu);
            actions.emplace_back(nullptr, itemIndex);
        } else {
            Widget *actionActor = Widget::createWidget<Action>(item.text, menuActor);
            Action *action = actionActor ? dynamic_cast<Action *>(actionActor) : nullptr;
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

    Font *font = Engine::loadResource<Font>(".embedded/Roboto.ttf");
    float offset = 0.0f;
    for(const auto &model : m_menuBar->menus()) {
        Widget *buttonActor = Widget::createWidget<Button>(model->title(), this);
        Button *button = buttonActor ? dynamic_cast<Button *>(buttonActor) : nullptr;
        if(!button) {
            continue;
        }

        button->setText(model->title());
        button->setFont(font);
        Widget *buttonRect = button;
        const float labelWidth = font ? font->textWidth(model->title(), button->fontSize(), 0) : 60.0f;
        const float width = labelWidth + kMenuHorizontalPadding * 2.0f;
        buttonRect->setPivot(Vector2(0.0f, 1.0f));
        buttonRect->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));
        buttonRect->setSize(Vector2(width, kMenuBarHeight));
        buttonRect->setPosition(Vector3(offset, 0.0f, 0.0f));
        offset += width;

        Menu *popup = buildMenu(model.get(), buttonActor, model->title() + TString("-popup"));
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
        Widget *childActor = dynamic_cast<Widget *>(child);
        if(childActor && dynamic_cast<Button *>(childActor)) {
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
    if(index == m_activeMenuIndex && m_topMenus[index]->isVisible()) {
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
}
