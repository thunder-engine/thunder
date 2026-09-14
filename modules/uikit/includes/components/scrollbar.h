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
#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "abstractslider.h"

class UIKIT_EXPORT ScrollBar : public AbstractSlider {
    A_OBJECT(ScrollBar, AbstractSlider, Components/UI)

    A_NOPROPERTIES()
    A_METHODS(
        A_SLOT(ScrollBar::stepBack),
        A_SLOT(ScrollBar::stepFront)
    )
    A_NOENUMS()

public:
    ScrollBar();

    void setValue(int value) override;
    void setMinimum(int value) override;
    void setMaximum(int value) override;

    void setOrientation(int orientation) override;

    int pageStep() const;
    void setPageStep(int page);

    int singleStep() const;
    void setSingleStep(int step);

    void stepBack();
    void stepFront();

    Widget *backArrow() const;
    void setBackArrow(Widget *arrow);

    Widget *frontArrow() const;
    void setFrontArrow(Widget *arrow);

private:
    void recalcKnob();

    void boundChanged(const Vector2 &size) override;

    void update(const Vector2 &pos) override;

    void composeComponent() override;

private:
    int m_pageStep;

    int m_singleStep;

};

#endif // SCROLLBAR_H
