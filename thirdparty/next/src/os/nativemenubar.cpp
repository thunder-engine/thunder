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

#include <os/menubar.h>

#include <log.h>

#include <utility>

// Forward declaration
bool menuBarNativeSupported();
bool menuBarNativeAttach(NativeMenuBar *menuBar, void *nativeWindow, void **nativeMenu);
void menuBarNativeDetach(void *nativeWindow, void *nativeMenu);
bool menuBarNativeRefresh(NativeMenuBar *menuBar, void *nativeWindow, void **nativeMenu);

#ifdef _WIN32
#include <windows.h>

#include <string>
#include <unordered_map>
#endif

bool triggerMenuShortcut(NativeMenu *menu, int key, int modifiers) {
    const auto &items = menu->items();
    for(size_t index = 0; index < items.size(); ++index) {
        const NativeMenu::Item &item = items[index];
        if(item.enabled && key != 0 &&
            item.shortcut == key && item.shortcutModifiers == modifiers) {
            menu->triggerAction(index);
            return true;
        }
        if(item.submenu && triggerMenuShortcut(item.submenu, key, modifiers)) {
            return true;
        }
    }
    return false;
}

#ifdef _WIN32
    struct WindowMenu {
        HMENU menu;
        WNDPROC previousProc;
        std::unordered_map<UINT_PTR, std::pair<NativeMenu *, size_t>> actions;
    };

    std::unordered_map<HWND, WindowMenu> g_windowMenus;
    UINT_PTR g_nextCommandId = 1;

    std::wstring toWide(const TString &text) {
        if(text.isEmpty()) {
            return std::wstring();
        }
        const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), -1, nullptr, 0);
        if(length <= 0) {
            return std::wstring();
        }
        std::wstring result(static_cast<size_t>(length), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.data(), -1, &result[0], length);
        result.resize(static_cast<size_t>(length - 1));
        return result;
    }

    bool appendMenuItems(HMENU nativeMenu, NativeMenu *menu, WindowMenu &state) {
        const auto &items = menu->items();
        for(size_t index = 0; index < items.size(); ++index) {
            const NativeMenu::Item &item = items[index];
            if(item.separator) {
                if(!AppendMenuW(nativeMenu, MF_SEPARATOR, 0, nullptr)) {
                    return false;
                }
            } else if(item.submenu) {
                HMENU submenu = CreatePopupMenu();
                if(!submenu || !appendMenuItems(submenu, item.submenu, state)) {
                    if(submenu) {
                        DestroyMenu(submenu);
                    }
                    return false;
                }
                const std::wstring title = toWide(item.submenu->title());
                if(!AppendMenuW(nativeMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(submenu), title.c_str())) {
                    DestroyMenu(submenu);
                    return false;
                }
            } else {
                UINT flags = MF_STRING;
                if(!item.enabled) {
                    flags |= MF_GRAYED;
                }
                if(item.checkable) {
                    flags |= item.checked ? MF_CHECKED : MF_UNCHECKED;
                }
                const UINT_PTR id = g_nextCommandId++;
                const std::wstring title = toWide(item.text);
                if(!AppendMenuW(nativeMenu, flags, id, title.c_str())) {
                    return false;
                }
                state.actions[id] = std::make_pair(menu, index);
            }
        }
        return true;
    }

    bool appendTopLevelMenus(HMENU nativeMenu, NativeMenuBar *menuBar, WindowMenu &state) {
        for(const auto &menu : menuBar->menus()) {
            HMENU popup = CreatePopupMenu();
            if(!popup || !appendMenuItems(popup, menu.get(), state)) {
                if(popup) {
                    DestroyMenu(popup);
                }
                return false;
            }
            const std::wstring title = toWide(menu->title());
            if(!AppendMenuW(nativeMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(popup), title.c_str())) {
                DestroyMenu(popup);
                return false;
            }
        }
        return true;
    }

    LRESULT CALLBACK menuWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        auto it = g_windowMenus.find(hwnd);
        if(message == WM_NCDESTROY && it != g_windowMenus.end()) {
            WNDPROC previousProc = it->second.previousProc;
            HMENU menu = it->second.menu;
            LRESULT result = previousProc ? CallWindowProcW(previousProc, hwnd, message, wParam, lParam) :
                                             DefWindowProcW(hwnd, message, wParam, lParam);
            g_windowMenus.erase(it);
            if(menu) {
                DestroyMenu(menu);
            }
            return result;
        }
        if(it != g_windowMenus.end() && message == WM_COMMAND && lParam == 0) {
            auto action = it->second.actions.find(LOWORD(wParam));
            if(action != it->second.actions.end()) {
                NativeMenu *menu = action->second.first;
                const size_t index = action->second.second;
                if(menu) {
                    menu->triggerAction(index);
                }
                return 0;
            }
        }
        if(it != g_windowMenus.end() && it->second.previousProc) {
            return CallWindowProcW(it->second.previousProc, hwnd, message, wParam, lParam);
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
#endif

NativeMenuBar::NativeMenuBar() :
        m_nativeWindow(nullptr),
        m_nativeMenu(nullptr),
        m_nextSubscription(1) {
}

NativeMenuBar::~NativeMenuBar() {
    detach();
}

NativeMenu *NativeMenuBar::addMenu(const TString &title) {
    std::unique_ptr<NativeMenu> menu(new NativeMenu(title));
    menu->setMenuBar(this);
    NativeMenu *result = menu.get();
    m_menus.push_back(std::move(menu));
    rebuild();
    return result;
}

const std::vector<std::unique_ptr<NativeMenu>> &NativeMenuBar::menus() const {
    return m_menus;
}

bool NativeMenuBar::attach(void *nativeWindow) {
    detach();
    if(!nativeWindow) {
        return false;
    }
    m_nativeWindow = nativeWindow;
    if(!menuBarNativeAttach(this, nativeWindow, &m_nativeMenu)) {
        m_nativeWindow = nullptr;
        m_nativeMenu = nullptr;
        return false;
    }
    return true;
}

void NativeMenuBar::detach() {
    if(m_nativeWindow) {
        menuBarNativeDetach(m_nativeWindow, m_nativeMenu);
        m_nativeWindow = nullptr;
        m_nativeMenu = nullptr;
    }
}

bool NativeMenuBar::isAttached() const {
    return m_nativeWindow != nullptr;
}

bool NativeMenuBar::isAttachedTo(void *nativeWindow) const {
    return nativeWindow && m_nativeWindow == nativeWindow;
}

bool NativeMenuBar::isNativeSupported() {
    return menuBarNativeSupported();
}

bool NativeMenuBar::triggerShortcut(int key, int modifiers) {
    for(const auto &menu : m_menus) {
        if(triggerMenuShortcut(menu.get(), key, modifiers)) {
            return true;
        }
    }
    return false;
}

NativeMenuBar::ChangeSubscription NativeMenuBar::subscribeChanged(const ChangedHandler &handler) {
    if(!handler) {
        return 0;
    }
    const ChangeSubscription subscription = m_nextSubscription++;
    m_changedHandlers.emplace(subscription, handler);
    return subscription;
}

void NativeMenuBar::unsubscribeChanged(ChangeSubscription subscription) {
    if(subscription != 0) {
        m_changedHandlers.erase(subscription);
    }
}

void NativeMenuBar::rebuild() {
    if(m_nativeWindow && !menuBarNativeRefresh(this, m_nativeWindow, &m_nativeMenu)) {
        aWarning() << "Failed to refresh the native menu bar; detaching it.";
        detach();
    }
    std::vector<ChangedHandler> handlers;
    handlers.reserve(m_changedHandlers.size());
    for(const auto &entry : m_changedHandlers) {
        handlers.push_back(entry.second);
    }
    for(const ChangedHandler &handler : handlers) {
        handler();
    }
}

#ifdef _WIN32
bool menuBarNativeSupported() {
    return true;
}

bool menuBarNativeAttach(NativeMenuBar *menuBar, void *nativeWindow, void **nativeMenu) {
    HWND hwnd = static_cast<HWND>(nativeWindow);
    if(!hwnd || g_windowMenus.find(hwnd) != g_windowMenus.end()) {
        return false;
    }
    WindowMenu state{CreateMenu(), nullptr, {}};
    if(!state.menu) {
        return false;
    }
    if(!appendTopLevelMenus(state.menu, menuBar, state)) {
        DestroyMenu(state.menu);
        return false;
    }

    SetLastError(ERROR_SUCCESS);
    state.previousProc = reinterpret_cast<WNDPROC>(
                SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(menuWindowProc)));
    if(!state.previousProc && GetLastError() != ERROR_SUCCESS) {
        DestroyMenu(state.menu);
        return false;
    }
    if(!SetMenu(hwnd, state.menu)) {
        SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(state.previousProc));
        DestroyMenu(state.menu);
        return false;
    }
    DrawMenuBar(hwnd);
    *nativeMenu = state.menu;
    g_windowMenus.emplace(hwnd, std::move(state));
    return true;
}

bool menuBarNativeRefresh(NativeMenuBar *menuBar, void *nativeWindow, void **nativeMenu) {
    HWND hwnd = static_cast<HWND>(nativeWindow);
    auto oldState = g_windowMenus.find(hwnd);
    if(oldState == g_windowMenus.end()) {
        return false;
    }

    WindowMenu state{CreateMenu(), oldState->second.previousProc, {}};
    if(!state.menu) {
        return false;
    }
    if(!appendTopLevelMenus(state.menu, menuBar, state)) {
        DestroyMenu(state.menu);
        return false;
    }
    HMENU oldMenu = oldState->second.menu;
    if(!SetMenu(hwnd, state.menu)) {
        DestroyMenu(state.menu);
        return false;
    }
    oldState->second = std::move(state);
    DrawMenuBar(hwnd);
    DestroyMenu(oldMenu);
    *nativeMenu = oldState->second.menu;
    return true;
}

void menuBarNativeDetach(void *nativeWindow, void *nativeMenu) {
    HWND hwnd = static_cast<HWND>(nativeWindow);
    auto it = g_windowMenus.find(hwnd);
    if(it == g_windowMenus.end()) {
        return;
    }
    SetMenu(hwnd, nullptr);
    DrawMenuBar(hwnd);
    DestroyMenu(it->second.menu);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(it->second.previousProc));
    g_windowMenus.erase(it);
    (void)nativeMenu;
}
#elif !defined(__APPLE__)
bool menuBarNativeSupported() {
    return false;
}

bool menuBarNativeAttach(NativeMenuBar *menuBar, void *nativeWindow, void **nativeMenu) {
    (void)menuBar;
    (void)nativeWindow;
    (void)nativeMenu;
    return false;
}

bool menuBarNativeRefresh(NativeMenuBar *menuBar, void *nativeWindow, void **nativeMenu) {
    (void)menuBar;
    (void)nativeWindow;
    (void)nativeMenu;
    return false;
}

void menuBarNativeDetach(void *nativeWindow, void *nativeMenu) {
    (void)nativeWindow;
    (void)nativeMenu;
}
#endif
