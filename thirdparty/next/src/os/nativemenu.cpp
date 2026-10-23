/*
    This file is part of Thunder Next.

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

#include <os/nativemenu.h>
#include <os/menubar.h>

#include <utility>

NativeMenu::NativeMenu(const TString &title) :
        m_title(title),
        m_menuBar(nullptr) {
}

NativeMenu::~NativeMenu() = default;

TString NativeMenu::title() const {
    return m_title;
}

void NativeMenu::setTitle(const TString &title) {
    if(m_title != title) {
        m_title = title;
        notifyChanged();
    }
}

NativeMenu::Item *NativeMenu::addAction(const TString &text, const std::function<void()> &handler,
                                        bool checkable, bool checked) {
    Item item;
    item.text = text;
    item.triggered = handler;
    item.checkable = checkable;
    item.checked = checkable && checked;
    m_items.push_back(std::move(item));
    notifyChanged();
    return &m_items.back();
}

void NativeMenu::setActionChecked(size_t index, bool checked) {
    if(index >= m_items.size()) {
        return;
    }
    Item &item = m_items[index];
    if(item.separator || item.submenu || !item.checkable || item.checked == checked) {
        return;
    }
    item.checked = checked;
    notifyChanged();
}

NativeMenu *NativeMenu::addMenu(const TString &title) {
    NativeMenu *menu = new NativeMenu(title);
    menu->setMenuBar(m_menuBar);
    Item item;
    item.text = title;
    item.submenu = menu;
    m_items.push_back(std::move(item));
    m_menus.push_back(menu);
    notifyChanged();
    return menu;
}

void NativeMenu::addSeparator() {
    Item item;
    item.separator = true;
    m_items.push_back(std::move(item));
    notifyChanged();
}

const std::deque<NativeMenu::Item> &NativeMenu::items() const {
    return m_items;
}

void NativeMenu::clear() {
    m_items.clear();
    m_menus.clear();
    notifyChanged();
}

void NativeMenu::triggerAction(size_t index) {
    if(index >= m_items.size()) {
        return;
    }
    Item &item = m_items[index];
    if(item.separator || item.submenu || !item.enabled) {
        return;
    }
    if(item.checkable) {
        item.checked = !item.checked;
        notifyChanged();
    }
    if(item.triggered) {
        item.triggered();
    }
}

void NativeMenu::setMenuBar(NativeMenuBar *bar) {
    m_menuBar = bar;
    for(const auto &menu : m_menus) {
        menu->setMenuBar(bar);
    }
}

void NativeMenu::notifyChanged() {
    if(m_menuBar) {
        m_menuBar->rebuild();
    }
}
