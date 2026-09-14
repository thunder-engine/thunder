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
#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "abstractbutton.h"

class UIKIT_EXPORT CheckBox : public AbstractButton {
    A_OBJECT(CheckBox, AbstractButton, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(Sprite *, indicator, CheckBox::indicator, CheckBox::setIndicator, "editor=Asset"),
        A_PROPERTYEX(Vector4, indicatorColor, CheckBox::indicatorColor, CheckBox::setIndicatorColor, "editor=Color"),
        A_PROPERTYEX(Vector2, iconSize, CheckBox::indicatorSize, CheckBox::setIndicatorSize, "css=indicator-size")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    CheckBox();

    Sprite *indicator() const;
    void setIndicator(Sprite *icon);

    Vector4 indicatorColor() const;
    void setIndicatorColor(const Vector4 &color);

    Vector2 indicatorSize() const;
    void setIndicatorSize(const Vector2 &size);

    void setFoldMode(bool fold);

protected:
    void composeComponent() override;

    void draw() override;

    void applyStyle() override;

    void boundChanged(const Vector2 &size) override;

protected:
    Vector4 m_knobColor;
    Vector2 m_knobSize;

    Sprite *m_knobIcon;

    Mesh *m_iconMesh;

    MaterialInstance *m_iconMaterial;
    MaterialInstance *m_frameMaterial;

    float m_iconOffset;

    bool m_foldMode;
    bool m_switchMode;
    bool m_dirtyIcon;

};

#endif // CHECKBOX_H
