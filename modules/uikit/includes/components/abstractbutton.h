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
#ifndef ABSTRACTBUTTON_H
#define ABSTRACTBUTTON_H

#include "frame.h"

class MaterialInstance;

class UIKIT_EXPORT AbstractButton : public Frame {
    A_OBJECT(AbstractButton, Frame, General)

    A_PROPERTIES(
        A_PROPERTY(bool, checkable, AbstractButton::isCheckable, AbstractButton::setCheckable),
        A_PROPERTY(bool, checked, AbstractButton::isChecked, AbstractButton::setChecked),
        A_PROPERTY(bool, exclusive, AbstractButton::isExclusive, AbstractButton::setExclusive)
    )
    A_METHODS(
        A_SIGNAL(AbstractButton::pressed),
        A_SIGNAL(AbstractButton::clicked),
        A_SIGNAL(AbstractButton::toggled)
    )
    A_NOENUMS()

public:
    AbstractButton();
    ~AbstractButton();

    Vector4 highlightedColor() const;
    void setHighlightedColor(const Vector4 &color);

    Vector4 pressedColor() const;
    void setPressedColor(const Vector4 &color);

    bool isCheckable() const;
    void setCheckable(bool checkable);

    bool isChecked() const;
    void setChecked(bool checked);

    bool isExclusive() const;
    void setExclusive(bool exclusive);

    void setHovered(bool hover, bool instant = false);

public: // signals
    void pressed();
    void clicked();

    void toggled(bool checked);

protected:
    void updateBackgroundColor(const Vector4 &color);

    void update(const Vector2 &pos) override;

    virtual void checkStateSet();

protected:
    Vector4 m_highlightedColor;
    Vector4 m_pressedColor;
    Vector4 m_currentColor;

    float m_fadeDuration;
    float m_currentFade;

    bool m_hovered;
    bool m_checkable;
    bool m_checked;
    bool m_exclusive;

};

#endif // ABSTRACTBUTTON_H
