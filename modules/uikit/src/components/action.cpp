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
#include "components/action.h"

#include <input.h>
#include <log.h>
#include <resources/sprite.h>

#include <algorithm>
#include <cctype>
#include <exception>
#include <string>

namespace {
    bool parseShortcut(const TString &shortcut, int &key, int &modifiers) {
        key = 0;
        modifiers = 0;
        if(shortcut.isEmpty()) {
            return true;
        }

        std::string value(shortcut.data());
        size_t start = 0;
        bool keyAssigned = false;
        auto assignKey = [&key, &keyAssigned](int value) {
            if(keyAssigned) {
                return false;
            }
            key = value;
            keyAssigned = true;
            return true;
        };

        while(start <= value.size()) {
            const size_t end = value.find('+', start);
            std::string token = value.substr(start, end == std::string::npos ? end : end - start);
            for(char &character : token) {
                character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
            }

            if(token == "CTRL" || token == "CONTROL") {
                modifiers |= Action::Control;
            } else if(token == "ALT") {
                modifiers |= Action::Alt;
            } else if(token == "SHIFT") {
                modifiers |= Action::Shift;
            } else if(token == "SUPER" || token == "META") {
                modifiers |= Action::Super;
            } else if(token.size() == 1 &&
                      ((token[0] >= 'A' && token[0] <= 'Z') || (token[0] >= '0' && token[0] <= '9'))) {
                if(!assignKey(token[0])) {
                    return false;
                }
            } else if(token.size() >= 2 && token[0] == 'F') {
                if(!std::all_of(token.begin() + 1, token.end(), [](char character) {
                    return character >= '0' && character <= '9';
                })) {
                    return false;
                }
                try {
                    const int functionKey = std::stoi(token.substr(1));
                    if(functionKey < 1 || functionKey > 25 || !assignKey(289 + functionKey)) {
                        return false;
                    }
                } catch(const std::exception &) {
                    return false;
                }
            } else {
                int specialKey = 0;
                if(token == "SPACE") {
                    specialKey = Input::KEY_SPACE;
                } else if(token == "ESC" || token == "ESCAPE") {
                    specialKey = Input::KEY_ESCAPE;
                } else if(token == "ENTER" || token == "RETURN") {
                    specialKey = Input::KEY_ENTER;
                } else if(token == "TAB") {
                    specialKey = Input::KEY_TAB;
                } else if(token == "BACKSPACE") {
                    specialKey = Input::KEY_BACKSPACE;
                } else if(token == "DELETE" || token == "DEL") {
                    specialKey = Input::KEY_DELETE;
                } else if(token == "LEFT") {
                    specialKey = Input::KEY_LEFT;
                } else if(token == "RIGHT") {
                    specialKey = Input::KEY_RIGHT;
                } else if(token == "UP") {
                    specialKey = Input::KEY_UP;
                } else if(token == "DOWN") {
                    specialKey = Input::KEY_DOWN;
                } else {
                    return false;
                }
                if(!assignKey(specialKey)) {
                    return false;
                }
            }

            if(end == std::string::npos) {
                break;
            }
            start = end + 1;
        }
        return keyAssigned;
    }
}

Action::Action() :
        m_icon(nullptr),
        m_shortcut(0),
        m_shortcutModifiers(0),
        m_enabled(true),
        m_checkable(false),
        m_checked(false) {
}

Action::~Action() {
    if(m_icon) {
        m_icon->unsubscribe(this);
    }
}

TString Action::text() const {
    return m_text;
}

void Action::setText(const TString &text) {
    if(m_text != text) {
        m_text = text;
        changed();
    }
}

bool Action::isEnabled() const {
    return m_enabled;
}

void Action::setEnabled(bool enabled) {
    if(m_enabled != enabled) {
        m_enabled = enabled;
        changed();
    }
}

bool Action::isCheckable() const {
    return m_checkable;
}

void Action::setCheckable(bool checkable) {
    if(m_checkable != checkable) {
        m_checkable = checkable;
        if(!checkable) {
            m_checked = false;
        }
        changed();
    }
}

bool Action::isChecked() const {
    return m_checked;
}

void Action::setChecked(bool checked) {
    if(m_checkable && m_checked != checked) {
        m_checked = checked;
        changed();
    }
}

Sprite *Action::icon() const {
    return m_icon;
}

void Action::setIcon(Sprite *icon) {
    if(m_icon != icon) {
        if(m_icon) {
            m_icon->unsubscribe(this);
        }
        m_icon = icon;
        if(m_icon) {
            m_icon->subscribe(&Action::iconUpdated, this);
        }
        changed();
    }
}

int Action::shortcut() const {
    return m_shortcut;
}

void Action::setShortcut(int key) {
    if(m_shortcut != key) {
        m_shortcut = key;
        changed();
    }
}

bool Action::setShortcutText(const TString &shortcut) {
    int key;
    int modifiers;
    if(!parseShortcut(shortcut, key, modifiers)) {
        aWarning() << "Invalid action shortcut:" << shortcut;
        return false;
    }
    setShortcut(key);
    setShortcutModifiers(modifiers);
    return true;
}

int Action::shortcutModifiers() const {
    return m_shortcutModifiers;
}

void Action::setShortcutModifiers(int modifiers) {
    if(m_shortcutModifiers != modifiers) {
        m_shortcutModifiers = modifiers;
        changed();
    }
}

TString Action::shortcutText() const {
    if(m_shortcut == 0) {
        return TString();
    }
    TString result;
    if(m_shortcutModifiers & Control) result += "Ctrl+";
    if(m_shortcutModifiers & Alt) result += "Alt+";
    if(m_shortcutModifiers & Shift) result += "Shift+";
    if(m_shortcutModifiers & Super) result += "Super+";

    if((m_shortcut >= 'A' && m_shortcut <= 'Z') || (m_shortcut >= '0' && m_shortcut <= '9')) {
        result += TString(std::string(1, static_cast<char>(m_shortcut)));
    } else if(m_shortcut >= 290 && m_shortcut <= 314) {
        result += TString("F") + TString::number(m_shortcut - 289);
    } else {
        switch(m_shortcut) {
            case 32: result += "Space"; break;
            case 256: result += "Esc"; break;
            case 257: result += "Enter"; break;
            case 258: result += "Tab"; break;
            case 259: result += "Backspace"; break;
            case 261: result += "Delete"; break;
            case 262: result += "Right"; break;
            case 263: result += "Left"; break;
            case 264: result += "Down"; break;
            case 265: result += "Up"; break;
            default: result += TString("Key ") + TString::number(m_shortcut); break;
        }
    }
    return result;
}

void Action::trigger() {
    if(!m_enabled) {
        return;
    }
    if(m_checkable) {
        m_checked = !m_checked;
        changed();
    }
    triggered();
}

void Action::triggered() {
    emitSignal(_SIGNAL(triggered()));
}

void Action::changed() {
    emitSignal(_SIGNAL(changed()));
}

void Action::iconUpdated(int state, void *ptr) {
    Action *action = static_cast<Action *>(ptr);
    if(!action) {
        return;
    }

    if(state == Resource::ToBeDeleted) {
        if(action->m_icon) {
            action->m_icon->unsubscribe(action);
        }
        action->m_icon = nullptr;
    }
    action->changed();
}
