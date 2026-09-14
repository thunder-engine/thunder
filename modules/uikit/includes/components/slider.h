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
#ifndef SLIDER_H
#define SLIDER_H

#include "abstractslider.h"

class ProgressBar;

class UIKIT_EXPORT Slider : public AbstractSlider {
    A_OBJECT(Slider, AbstractSlider, Components/UI)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    Slider();

    void setOrientation(int value) override;

    ProgressBar *background() const;
    void setBackground(ProgressBar *background);

    void setValue(int value) override;
    void setMinimum(int value) override;
    void setMaximum(int value) override;

private:
    void update(const Vector2 &pos) override;

    void composeComponent() override;

private:
    Vector4 m_normalColor;

    Vector4 m_highlightedColor;

};

#endif // SLIDER_H
