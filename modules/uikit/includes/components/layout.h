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
#ifndef LAYOUT_H
#define LAYOUT_H

#include "widget.h"

class UIKIT_EXPORT Layout {
public:
    Layout();
    ~Layout();

    void addTransform(RectTransform *transform);

    void insertTransform(int index, RectTransform *transform);

    void removeTransform(RectTransform *transform);

    int indexOf(const RectTransform *transform) const;

    RectTransform *transformAt(int index);

    RectTransform *rectTransform();
    void setRectTransform(RectTransform *transform);

    int count() const;

    int spacing() const;
    void setSpacing(int spacing);

    int orientation() const;
    void setOrientation(int orientation);

    Vector2 sizeHint();

    void invalidate();

    void solveItemsDimension(float availableSpace, bool horizontal, bool keepProportions);
    void solveItemsPosition(float height, const Vector2 &offset);

protected:
    std::list<RectTransform *> m_items;

    RectTransform *m_rectTransform;

    int m_spacing;

    int m_orientation;
};

#endif // LAYOUT_H
