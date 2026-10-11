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

#ifndef NATIVEMENU_H
#define NATIVEMENU_H

#include <astring.h>

#include <cstddef>
#include <deque>
#include <functional>

class NativeMenuBar;

class NEXT_LIBRARY_EXPORT NativeMenu {
public:
    struct Item {
        TString text;
        std::function<void()> triggered;
        int shortcut = 0;
        int shortcutModifiers = 0;
        bool enabled = true;
        bool checkable = false;
        bool checked = false;
        bool separator = false;
        NativeMenu *submenu = nullptr;
    };

    explicit NativeMenu(const TString &title = TString());
    ~NativeMenu();

    TString title() const;
    void setTitle(const TString &title);

    Item *addAction(const TString &text, const std::function<void()> &handler = std::function<void()>());
    NativeMenu *addMenu(const TString &title);
    void addSeparator();

    const std::deque<Item> &items() const;
    void clear();
    void triggerAction(size_t index);

private:
    friend class NativeMenuBar;

    void setMenuBar(NativeMenuBar *bar);
    void notifyChanged();

    TString m_title;
    NativeMenuBar *m_menuBar;
    std::deque<Item> m_items;
    std::vector<NativeMenu *> m_menus;
};

#endif // NATIVEMENU_H
