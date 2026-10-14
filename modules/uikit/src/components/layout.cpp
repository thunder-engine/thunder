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
#include "components/layout.h"

/*!
    \class Layout
    \brief The Layout class is a base class for managing the layout and positioning of widgets within a graphical user interface.
    \inmodule Gui

    The Layout class is a base class used for managing the layout and positioning of widgets within a graphical user interface (GUI).
    It provides a structured way to organize UI elements, ensuring that they are placed efficiently and consistently on the screen.
    The Layout class is essential for developers who need to arrange multiple components (such as buttons, labels, text fields, etc.) in a clean and organized manner.
*/

Layout::Layout() :
        m_widget(nullptr),
        m_spacing(0),
        m_orientation(Widget::Vertical) {

}

Layout::~Layout() {
    for(auto it : m_items) {
        it->m_attachedLayout = nullptr;
    }
}
/*!
    Adds a \a transform to the current layout.
*/
void Layout::addWidget(Widget *widget) {
    insertWidget(-1, widget);
}
/*!
     Inserts a \a transform at the specified \a index.
     If -1, the layout is appended to the end.
*/
void Layout::insertWidget(int index, Widget *widget) {
    if(widget) {
        if(widget->m_attachedLayout) {
            widget->m_attachedLayout->removeWidget(widget);
            widget->m_attachedLayout = nullptr;
        }

        widget->m_attachedLayout = this;
        widget->setPivot(Vector2(0, 1));

        if(index >= 0 && index < m_items.size()) {
            m_items.insert(std::next(m_items.begin(), index), widget);
        } else {
            m_items.push_back(widget);
        }
        if(m_widget && widget->parentWidget() != m_widget) {
            widget->setParent(m_widget, index);
        }

        invalidate();
        if(m_widget) {
            m_widget->cleanDirty();
        }
    }
}
/*!
    Removes a \a transform from the current layout.
*/
void Layout::removeWidget(Widget *widget) {
    if(widget) {
        m_items.remove(widget);
        if(widget->m_attachedLayout == this) {
            widget->m_attachedLayout = nullptr;
        }

        invalidate();
    }
}
/*!
    Returns the index of the specified \a transform.
*/
int Layout::indexOf(const Widget *widget) const {
    int result = -1;
    for(auto it : m_items) {
        ++result;
        if(it == widget) {
            return result;
        }
    }
    return -1;
}
/*!
    Returns transform located at \a index.
*/
Widget *Layout::widgetAt(int index) {
    if(index > -1 && index < m_items.size()) {
        auto it = std::next(m_items.begin(), index);
        return (*it);
    }
    return nullptr;
}
/*!
    Returns the parent rect transform of this layout, or nullptr if this layout is not installed on any rect transform.
    If the layout is a sub-layout, this function returns the parent rect transform of the parent layout.
*/
Widget *Layout::widget() const {
    return m_widget;
}
/*!
    \internal
    Sets the parent rect \a tranform for this layout.
    It will be used to notify parent rect transform for any layout changes.
*/
void Layout::setWidget(Widget *widget) {
    m_widget = widget;
    invalidate();
}
/*!
    Returns number of items in the layout.
*/
int Layout::count() const {
    return m_items.size();
}
/*!
    Returns the spacing between items in the layout.
*/
int Layout::spacing() const {
    return m_spacing;
}
/*!
    Sets the \a spacing between items in the layout.
*/
void Layout::setSpacing(int spacing) {
    m_spacing = spacing;
    invalidate();
}
/*!
    Returns the layout orientation (Vertical or Horizontal).
*/
int Layout::orientation() const {
    return m_orientation;
}
/*!
    Sets the layout \a orientation.
*/
void Layout::setOrientation(int orientation) {
    m_orientation = orientation;
    invalidate();
}
/*!
    Returns the size hint for the layout.
*/
Vector2 Layout::sizeHint() {
    Vector2 result;

    bool first = true;
    for(auto it : m_items) {
        it->cleanDirtySize();

        if(it->isEnabled() && it->isVisible()) {
            float spacing = (!first) ? m_spacing : 0.0f;

            Vector4 margin(it->margin());
            Vector2 size(it->sizeHint());
            size.x += (margin.w + margin.y);
            size.y += (margin.x + margin.z);

            if(m_orientation == Widget::Vertical) {
                result.x = MAX(result.x, size.x);
                result.y += spacing + size.y;
            } else {
                result.x += spacing + size.x;
                result.y = MAX(result.y, size.y);
            }

            first = false;
        }
    }

    if(m_widget) {
        Vector4 padding(m_widget->padding());
        Vector4 border(m_widget->border());

        result.x += padding.w + padding.y + border.y + border.w;
        result.y += padding.x + padding.z + border.x + border.z;
    }

    return result;
}
/*!
    Marks the layout as dirty, indicating that it needs to be recomputed.
*/
void Layout::invalidate() {
    if(m_widget) {
        m_widget->setDirty();
        m_widget->repaint();

        Layout *layout = m_widget->m_attachedLayout;
        if(layout) {
            layout->invalidate();
        }
    }
}
/*!
    \internal
*/
void Layout::solveItemsDimension(float availableSpace, bool horizontal, bool keepProportions) {
    if(m_orientation == Widget::Vertical) {
        if(horizontal) {
            for(auto it : m_items) {
                if(it->horizontalPolicy() == Widget::Expanding) {
                    Vector4 margin(it->margin());
                    Vector2 size(it->size());
                    size.x = availableSpace - (margin.w + margin.y);
                    it->blockSignals(true);
                    it->setSize(size);
                    it->blockSignals(false);
                }
            }
            return;
        }
    } else {
        if(!horizontal) {
            for(auto it : m_items) {
                if(it->verticalPolicy() == Widget::Expanding) {
                    Vector4 margin(it->margin());
                    Vector2 size(it->size());
                    size.y = availableSpace - (margin.x + margin.z);
                    it->blockSignals(true);
                    it->setSize(size);
                    it->blockSignals(false);
                }
            }
            return;
        }
    }

    float totalPref = 0.0f;
    float totalExp = 0.0f;
    int expandingCount = 0;
    int preferredCount = 0;
    int fixedCount = 0;

    std::list<float> sizes;

    for(auto it : m_items) {
        if(it->isEnabled() && it->isVisible()) {
            Widget::SizePolicy policy = static_cast<Widget::SizePolicy>(horizontal ? it->horizontalPolicy() : it->verticalPolicy());
            float pref = horizontal ? it->size().x : it->size().y;
            Vector4 margin(it->margin());
            pref += horizontal ? (margin.w + margin.y) : (margin.x + margin.z);

            totalPref += pref;

            switch(policy) {
                case Widget::Fixed: fixedCount++; break;
                case Widget::Preferred: preferredCount++; break;
                case Widget::Expanding: expandingCount++; totalExp += pref; sizes.push_back(pref); break;
                default: break;
            }
        }
    }

    int spacesCount = MAX(expandingCount + preferredCount + fixedCount - 1, 0);

    totalPref += spacesCount * m_spacing;

    auto size = sizes.begin();
    std::list<float> weights;
    for(auto it : m_items) {
        if(it->isEnabled() && it->isVisible()) {
            Widget::SizePolicy policy = static_cast<Widget::SizePolicy>(horizontal ? it->horizontalPolicy() : it->verticalPolicy());
            if(policy == Widget::Expanding) {
                float weight = 1.0f / static_cast<float>(expandingCount);
                if(keepProportions && totalExp > 0.0f) {
                    weight = (*size) / totalExp;
                }
                weights.push_back(weight);
                ++size;
            }
        }
    }

    if(availableSpace <= totalPref) { // Need to reduce size of widgets
        float remainingSpace = availableSpace;

        for(auto it : m_items) {
            if(it->isEnabled() && it->isVisible()) {
                Widget::SizePolicy policy = static_cast<Widget::SizePolicy>(horizontal ? it->horizontalPolicy() : it->verticalPolicy());
                if(policy == Widget::Fixed || policy == Widget::Preferred) {
                    float pref = horizontal ? it->size().x : it->size().y;
                    Vector4 margin(it->margin());
                    pref += horizontal ? (margin.w + margin.y) : (margin.x + margin.z);
                    remainingSpace -= pref;
                }
            }
        }

        remainingSpace -= spacesCount * m_spacing;

        if(expandingCount > 0 && remainingSpace > 0) {
            auto weight = weights.begin();
            for(auto it : m_items) {
                if(it->isEnabled() && it->isVisible()) {
                    Widget::SizePolicy policy = static_cast<Widget::SizePolicy>(horizontal ? it->horizontalPolicy() : it->verticalPolicy());
                    if(policy == Widget::Expanding) {
                        Vector4 margin(it->margin());
                        Vector2 size(it->size());
                        if(horizontal) {
                            size.x = remainingSpace * (*weight) - (margin.w + margin.y);
                        } else {
                            size.y = remainingSpace * (*weight) - (margin.x + margin.z);
                        }
                        it->blockSignals(true);
                        it->setSize(size);
                        it->blockSignals(false);
                        ++weight;
                    }
                }
            }
        }
    } else { // Need to expand widgets
        float extra = availableSpace - totalPref;
        auto weight = weights.begin();
        for(auto it : m_items) {
            if(it->isEnabled() && it->isVisible()) {
                Widget::SizePolicy policy = static_cast<Widget::SizePolicy>(horizontal ? it->horizontalPolicy() : it->verticalPolicy());
                if(policy == Widget::Expanding) {
                    float extraSize = extra * (*weight);
                    if(extraSize != 0) {
                        Vector2 size(it->size());
                        if(horizontal) {
                            size.x += extraSize;
                        } else {
                            size.y += extraSize;
                        }
                        it->blockSignals(true);
                        it->setSize(size);
                        it->blockSignals(false);
                    }
                    ++weight;
                }
            }
        }
    }
}
/*!
    \internal
*/
void Layout::solveItemsPosition(float height, const Vector2 &offset) {
    bool first = true;
    float shift = ((m_orientation == Widget::Vertical) ? offset.y : offset.x);
    for(auto it : m_items) {
        if(it->isEnabled() && it->isVisible()) {
            shift += (!first) ? m_spacing : 0.0f;

            Vector2 size(it->size());
            Vector2 pivot(it->pivot());
            Vector4 margin(it->margin());

            if(m_orientation == Widget::Vertical) {
                shift += size.y * pivot.y + margin.z;
                it->m_position.x = offset.x + size.x * pivot.x + margin.w;
                it->m_position.y = height - shift;
                shift += size.y * (1.0f - pivot.y) + margin.x;
            } else {
                shift += size.x * pivot.x + margin.w;
                it->m_position.x = shift;
                it->m_position.y = offset.y - size.y * (1.0f - pivot.y) - margin.z;
                shift += size.x * (1.0f - pivot.x) + margin.y;
            }
            it->setDirty();

            first = false;
        }
    }
}
