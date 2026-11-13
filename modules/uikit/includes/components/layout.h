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

#include <widget.h>

class UIKIT_EXPORT Layout {
public:
    Layout();
    ~Layout();

    void addWidget(Widget *widget);

    void insertWidget(int index, Widget *widget);

    void removeWidget(Widget *widget);

    int indexOf(const Widget *widget) const;

    Widget *widgetAt(int index);

    Widget *widget() const;
    void setWidget(Widget *widget);

    void addTransform(Widget *widget) { addWidget(widget); }
    void insertTransform(int index, Widget *widget) { insertWidget(index, widget); }
    void removeTransform(Widget *widget) { removeWidget(widget); }
    Widget *transformAt(int index) { return widgetAt(index); }

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
    std::list<Widget *> m_items;

    Widget *m_widget;

    int m_spacing;

    int m_orientation;
};

#endif // LAYOUT_H
