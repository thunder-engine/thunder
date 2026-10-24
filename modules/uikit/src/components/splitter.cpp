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
#include "splitter.h"

#include "layout.h"

#include <input.h>

#include <utility>
#include <vector>

/*!
    \class Splitter
    \brief A splitter widget that allows resizing of child widgets.
    \inmodule Gui

    Splitter provides a container that divides its area into resizable panes.
    Users can drag the splitter handles to adjust the size of adjacent child widgets.
    Supports both horizontal and vertical orientations.
*/

Splitter::Splitter() :
        m_savedPosition(0.0f),
        m_index(-1),
        m_orientation(Widget::Horizontal),
        m_handleWidth(5),
        m_initialShare(0.5f),
        m_initialSharePending(false) {

}
/*!
    Returns the width (or thickness) of the splitter handle.
*/
int Splitter::handleWidth() const {
    return m_handleWidth;
}
/*!
    Sets the \a width (or thickness) of the splitter handle.
*/
void Splitter::setHandleWidth(int width) {
    m_handleWidth = width;
    Layout *layout = Splitter::layout();
    if(layout) {
        layout->setSpacing(width);
        repaint();
    }
}
/*!
    Returns the current orientation of the splitter.
*/
int Splitter::orentation() const {
    return m_orientation;
}
/*!
    Sets the \a orientation of the splitter.
*/
void Splitter::setOrientation(int orientation) {
    m_orientation = orientation;
    Layout *layout = Splitter::layout();
    if(layout) {
        layout->setOrientation(m_orientation);

        if(m_orientation == Widget::Horizontal) {
            for(int i = 0; i < layout->count(); i++) {
                Widget *r = layout->widgetAt(i);
                r->setVerticalPolicy(Widget::Expanding);
                if(i == layout->count() - 1) {
                    r->setHorizontalPolicy(Widget::Expanding);
                } else {
                    r->setHorizontalPolicy(Widget::Fixed);
                }
            }

        } else {
            for(int i = 0; i < layout->count(); i++) {
                Widget *r = layout->widgetAt(i);
                r->setHorizontalPolicy(Widget::Expanding);
                if(i == layout->count() - 1) {
                    r->setVerticalPolicy(Widget::Expanding);
                } else {
                    r->setVerticalPolicy(Widget::Fixed);
                }
            }
        }
    }
}

void Splitter::setInitialShare(float share) {
    m_initialShare = std::max(0.05f, std::min(share, 0.95f));
    m_initialSharePending = true;

    const Vector2 currentSize = size();
    const float extent = m_orientation == Widget::Horizontal ? currentSize.x : currentSize.y;
    if(extent > 0.0f) {
        boundChanged(currentSize);
    }
}

/*!
    Adds a \a widget to the splitter at the end.
*/
void Splitter::addWidget(Widget *widget) {
    insertWidget(-1, widget);
}
/*!
    Returns the number of child widgets in the splitter.
*/
int Splitter::count() {
    Layout *layout = Splitter::layout();
    return layout ? layout->count() : 0;
}
/*!
    Returns the index of a child \a widget.
*/
int Splitter::indexOf(Widget *widget) {
    Layout *layout = Splitter::layout();
    if(layout && widget) {
        return layout->indexOf(widget);
    }

    return -1;
}
/*!
    Inserts a \a widget at the specified \a index.
*/
void Splitter::insertWidget(int index, Widget *widget) {
    if(!widget) {
        return;
    }

    Layout *layout = Splitter::layout();
    widget->setParent(this);
    if(layout) {
        layout->insertWidget(index, widget);
        setOrientation(m_orientation);
    }
    repaint();
}
/*!
    Replaces a \a widget at the specified \a index.
*/
Widget *Splitter::replaceWidget(int index, Widget *widget) {
    Widget *result = Splitter::widget(index);
    if(result) {
        Layout *layout = Splitter::layout();
        if(layout) {
            layout->removeWidget(result);
        }

        if(widget) {
            insertWidget(index, widget);
        }
        repaint();
    }

    return result;
}
/*!
    Returns the widget at the specified \a index.
*/
Widget *Splitter::widget(int index) {
    Layout *layout = Splitter::layout();
    if(layout) {
        Widget *rect = layout->widgetAt(index);
        if(rect) {
            return rect;
        }
    }

    return nullptr;
}
/*!
    \internal
*/
void Splitter::update(const Vector2 &pos) {
    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        m_index = -1;
    }

    if(isHovered(pos)) {
        Input::CursorShape shape = Input::CURSOR_ARROW;

        Layout *layout = Splitter::layout();
        if(layout) {
            if(m_index == -1) {
                const float sense = layout->spacing() * 2;
                for(int i = 0; i < count()-1; i++) {
                    Widget *child = layout->widgetAt(i);
                    const Matrix4 m(child->worldTransform());
                    const Vector2 size(child->size() * child->worldScale());
                    if(m_orientation == Widget::Horizontal) {
                        float d = std::abs((m[12] + size.x) - pos.x);
                        if(d < sense) {
                            shape = Input::CURSOR_HORSIZE;
                            break;
                        }
                    } else {
                        float d = std::abs(m[13] - pos.y);
                        if(d < sense) {
                            shape = Input::CURSOR_VERSIZE;
                            break;
                        }
                    }
                }

            } else {
                shape = (m_orientation == Widget::Horizontal) ? Input::CURSOR_HORSIZE : Input::CURSOR_VERSIZE;

                if(Input::isMouseButton(Input::MOUSE_LEFT)) {
                    float delta = (m_orientation == Widget::Horizontal) ? (pos.x - m_savedPosition) : (m_savedPosition - pos.y);
                    if(delta != 0.0f) {
                        m_savedPosition = (m_orientation == Widget::Horizontal) ? pos.x : pos.y;

                        resizeWidget(m_index, delta);
                        resizeWidget(m_index + 1, -delta);
                    }
                }

            }
        }

        Input::mouseSetCursor(shape);
    }

    Widget::update(pos);
}

bool Splitter::onMouseDown(int x, int y) {
    A_UNUSED(x);
    A_UNUSED(y);
    Layout *layout = Splitter::layout();
    if(!layout) {
        return false;
    }

    const Vector2 pos = mousePosition();
    const float sense = layout->spacing() * 2.0f;
    for(int i = 0; i < count() - 1; ++i) {
        Widget *child = layout->widgetAt(i);
        if(!child) {
            continue;
        }
        const Matrix4 transform(child->worldTransform());
        const Vector2 childSize(child->size() * child->worldScale());
        const float distance = m_orientation == Widget::Horizontal ?
            std::abs((transform[12] + childSize.x) - pos.x) : std::abs(transform[13] - pos.y);
        if(distance < sense) {
            m_savedPosition = m_orientation == Widget::Horizontal ? pos.x : pos.y;
            m_index = i;
            return true;
        }
    }
    return false;
}

bool Splitter::onMouseUp(int x, int y) {
    A_UNUSED(x);
    A_UNUSED(y);
    const bool wasDragging = m_index > -1;
    m_index = -1;
    return wasDragging;
}

void Splitter::boundChanged(const Vector2 &size) {
    Layout *layout = Splitter::layout();
    if(m_initialSharePending && layout && layout->count() >= 2) {
        const bool horizontal = m_orientation == Widget::Horizontal;
        const float extent = horizontal ? size.x : size.y;
        const float available = MAX(0.0f, extent - layout->spacing() * (layout->count() - 1));
        if(available > 0.0f) {
            const float remainingShare = (1.0f - m_initialShare) / (layout->count() - 1);
            for(int i = 0; i < layout->count(); ++i) {
                Widget *child = layout->widgetAt(i);
                if(!child) {
                    continue;
                }

                Vector2 childSize = child->size();
                const float childShare = i == 0 ? m_initialShare : remainingShare;
                if(horizontal) {
                    childSize.x = available * childShare;
                } else {
                    childSize.y = available * childShare;
                }
                child->setSize(childSize);
            }
            m_initialSharePending = false;
        }
    }

    Frame::boundChanged(size);

    if(!layout || layout->count() < 2) {
        return;
    }

    const bool horizontal = m_orientation == Widget::Horizontal;
    const Vector4 padding = Splitter::padding();
    const Vector4 border = Splitter::border();
    const float leading = horizontal ? padding.w + border.w : padding.x + border.x;
    const float trailing = horizontal ? padding.y + border.y : padding.z + border.z;
    const float innerSize = MAX(0.0f, (horizontal ? size.x : size.y) - leading - trailing);

    std::vector<std::pair<Widget *, float>> children;
    float totalSize = 0.0f;
    for(int i = 0; i < layout->count(); ++i) {
        Widget *child = layout->widgetAt(i);
        if(!child || !child->isVisible()) {
            continue;
        }
        Vector4 margin = child->margin();
        const float margins = horizontal ? margin.w + margin.y : margin.x + margin.z;
        totalSize += (horizontal ? child->size().x : child->size().y) + margins;
        children.emplace_back(child, margins);
    }

    if(children.empty()) {
        return;
    }
    const float available = MAX(0.0f, innerSize - layout->spacing() *
                                static_cast<float>(children.size() - 1));
    if(available <= 0.0f || totalSize <= available || totalSize <= 0.0f) {
        return;
    }

    const float scale = available / totalSize;
    for(const auto &entry : children) {
        Widget *child = entry.first;
        Vector2 childSize = child->size();
        if(horizontal) {
            childSize.x = MAX(0.0f, (childSize.x + entry.second) * scale - entry.second);
        } else {
            childSize.y = MAX(0.0f, (childSize.y + entry.second) * scale - entry.second);
        }
        child->setSize(childSize);
    }
}

/*!
    \internal
*/
void Splitter::resizeWidget(int index, float delta) {
    Widget *widget = Splitter::widget(index);
    if(widget) {
        Widget *rect = widget;
        Vector2 size(rect->size());
        if(m_orientation == Widget::Horizontal) {
            size.x = MAX(size.x + delta, 0.0f);
        } else {
            size.y = MAX(size.y + delta, 0.0f);
        }
        rect->setSize(size);
        widget->repaint();
    }
}
/*!
    \internal
*/
void Splitter::childAdded(Widget *rect) {
    if(rect) {
        Layout *layout = Splitter::layout();
        if(m_orientation == Widget::Horizontal) {
            rect->setVerticalPolicy(Widget::Expanding);
            for(int i = 0; i < layout->count(); i++) {
                Widget *r = layout->widgetAt(i);
                if(r == rect) {
                    rect->setHorizontalPolicy(Widget::Expanding);
                } else {
                    r->setHorizontalPolicy(Widget::Fixed);
                }
            }
        } else {
            rect->setHorizontalPolicy(Widget::Expanding);
            for(int i = 0; i < layout->count(); i++) {
                Widget *r = layout->widgetAt(i);
                if(r == rect) {
                    rect->setVerticalPolicy(Widget::Expanding);
                } else {
                    r->setVerticalPolicy(Widget::Fixed);
                }
            }
        }
    }
}
/*!
    \internal
*/
void Splitter::composeComponent() {
    Frame::composeComponent();

    Layout *layout = new Layout;
    layout->setOrientation(m_orientation);
    layout->setSpacing(5);

    setLayout(layout);
}
