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

#include <string>

Action::Action() :
        m_shortcut(0),
        m_shortcutModifiers(0),
        m_checkable(false),
        m_checked(false) {
}

TString Action::text() const {
    return m_text;
}

void Action::setText(const TString &text) {
    if(m_text != text) {
        m_text = text;
        changed();
        repaint();
    }
}

bool Action::isEnabled() const {
    return Widget::isEnabled();
}

void Action::setEnabled(bool enabled) {
    if(isEnabled() != enabled) {
        Widget::setEnabled(enabled);
        changed();
        repaint();
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
        repaint();
    }
}

bool Action::isChecked() const {
    return m_checked;
}

void Action::setChecked(bool checked) {
    if(m_checkable && m_checked != checked) {
        m_checked = checked;
        changed();
        repaint();
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

void Action::setTriggeredHandler(const TriggeredHandler &handler) {
    m_handler = handler;
}

void Action::trigger() {
    if(!isEnabled()) {
        return;
    }
    if(m_checkable) {
        m_checked = !m_checked;
        changed();
    }
    triggered();
    if(m_handler) {
        m_handler();
    }
}

void Action::triggered() {
    emitSignal(_SIGNAL(triggered()));
}

void Action::changed() {
    emitSignal(_SIGNAL(changed()));
}
