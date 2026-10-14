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
#ifndef ACTION_H
#define ACTION_H

#include <widget.h>

#include <functional>

class UIKIT_EXPORT Action : public Widget {
    A_OBJECT(Action, Widget, UI)

    A_PROPERTIES(
        A_PROPERTY(TString, text, Action::text, Action::setText),
        A_PROPERTY(bool, checkable, Action::isCheckable, Action::setCheckable),
        A_PROPERTY(bool, checked, Action::isChecked, Action::setChecked),
        A_PROPERTY(int, shortcut, Action::shortcut, Action::setShortcut),
        A_PROPERTY(int, shortcutModifiers, Action::shortcutModifiers, Action::setShortcutModifiers)
    )
    A_METHODS(
        A_SIGNAL(Action::triggered),
        A_SIGNAL(Action::changed)
    )
    A_NOENUMS()

public:
    using TriggeredHandler = std::function<void()>;

    enum ShortcutModifier {
        Shift = 0x0001,
        Control = 0x0002,
        Alt = 0x0004,
        Super = 0x0008
    };

    Action();
    ~Action() override = default;

    TString text() const;
    void setText(const TString &text);
    void setEnabled(bool enabled) override;

    bool isCheckable() const;
    void setCheckable(bool checkable);

    bool isChecked() const;
    void setChecked(bool checked);

    int shortcut() const;
    void setShortcut(int key);

    int shortcutModifiers() const;
    void setShortcutModifiers(int modifiers);
    TString shortcutText() const;

    void setTriggeredHandler(const TriggeredHandler &handler);
    void trigger();

    void triggered();
    void changed();

private:
    TString m_text;
    TriggeredHandler m_handler;
    int m_shortcut;
    int m_shortcutModifiers;
    bool m_checkable;
    bool m_checked;
};

#endif // ACTION_H
