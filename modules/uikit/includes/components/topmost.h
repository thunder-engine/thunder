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
#ifndef TOPMOST_H
#define TOPMOST_H

#include <uikit.h>
#include <components/frame.h>

class UIKIT_EXPORT TopMost : public Frame {
    A_OBJECT(TopMost, Frame, UI)
    A_PROPERTIES(
        A_PROPERTYEX(Level, level, TopMost::level, TopMost::setLevel, "enum=Level")
    )
    A_ENUMS(
        A_ENUM(Level,
            A_VALUE(DialogLevel),
            A_VALUE(MenuLevel),
            A_VALUE(HintLevel)
        )
    )
public:
    enum Level {
        DialogLevel,
        MenuLevel,
        HintLevel
    };

    TopMost();
    ~TopMost() override;

    Level level() const;
    void setLevel(Level level);
    bool show(Widget *anchor, const Vector2 &offset = Vector2());
    virtual void hide();

    void draw() override;

private:
    Level m_level = DialogLevel;
};

#endif // TOPMOST_H
