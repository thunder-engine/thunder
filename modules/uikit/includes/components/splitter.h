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
#ifndef SPLITTER_H
#define SPLITTER_H

#include "frame.h"

class UIKIT_EXPORT Splitter : public Frame {
    A_OBJECT(Splitter, Frame, Components/UI)

    A_PROPERTIES(
        A_PROPERTYEX(int, orentation, Splitter::orentation, Splitter::setOrientation, "enum=Orientation"),
        A_PROPERTY(int, handleWidth, Splitter::handleWidth, Splitter::setHandleWidth)
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    Splitter();

    int handleWidth() const;
    void setHandleWidth(int width);

    int orentation() const;
    void setOrientation(int orientation);

    void addWidget(Widget *widget);

    int count();

    int indexOf(Widget *widget);

    void insertWidget(int index, Widget *widget);

    Widget *replaceWidget(int index, Widget *widget);

    Widget *widget(int index);

protected:
    void update(const Vector2 &pos) override;

    void resizeWidget(int index, float delta);

    void childAdded(RectTransform *rect) override;

    void composeComponent() override;

protected:
    float m_savedPosition;

    int m_index;

    int m_orientation;

    int m_handleWidth;

};

#endif // SPLITTER_H
