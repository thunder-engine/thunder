#include "components/gui/layout.h"

#include "components/gui/recttransform.h"
#include "components/actor.h"

/*!
    \class Layout
    \brief The Layout class is a base class for managing the layout and positioning of widgets within a graphical user interface.
    \inmodule Gui

    The Layout class provides a flexible mechanism for managing the arrangement of widgets and child layouts within a graphical user interface.
    Child items can be widgets or nested layouts, and the layout can be configured with spacing, margins, and direction.
*/

Layout::Layout() :
        m_attachedWidget(nullptr),
        m_parentLayout(nullptr),
        m_spacing(0.0f),
        m_direction(Vertical),
        m_dirty(false) {

}

Layout::~Layout() {
    for(auto it : m_items) {
        if(it->m_attachedWidget) {
            it->m_attachedWidget->m_attachedLayout = nullptr;
        }
    }
}
/*!
    Adds a child \a layout to the current layout.
*/
void Layout::addLayout(Layout *layout) {
    insertLayout(-1, layout);
}
/*!
    Adds a \a widget to the current layout.
*/
void Layout::addWidget(Widget *widget) {
    insertWidget(-1, widget);
}
/*!
    Inserts a child \a layout at the specified \a index.
    If -1, the layout is appended to the end.
*/
void Layout::insertLayout(int index, Layout *layout) {
    if(layout) {
        layout->m_parentLayout = this;
        if(index > 0) {
            m_items.insert(std::next(m_items.begin(), index), layout);
        } else {
            m_items.push_back(layout);
        }
        invalidate();
    }
}
/*!
     Inserts a \a widget at the specified \a index.
     If -1, the layout is appended to the end.
*/
void Layout::insertWidget(int index, Widget *widget) {
    if(widget) {
        Layout *layout = new Layout;
        layout->m_attachedWidget = widget;
        widget->m_attachedLayout = this;

        insertLayout(index, layout);
    }
}
/*!
    Removes a child \a layout from the current layout.
*/
void Layout::removeLayout(Layout *layout) {
    m_items.remove(layout);
    invalidate();
}
/*!
    Removes a \a widget from the current layout.
*/
void Layout::removeWidget(Widget *widget) {
    for(auto it : m_items) {
        if(it->m_attachedWidget == widget) {
            Layout *tmp = it;
            m_items.remove(tmp);
            delete tmp;
            invalidate();

            break;
        }
    }
}
/*!
    Returns the index of the specified child \a layout.
*/
int Layout::indexOf(const Layout *layout) const {
    int result = -1;
    for(auto it : m_items) {
        ++result;
        if(it == layout) {
            break;
        }
    }
    return result;
}
/*!
    Returns the index of the specified \a widget.
*/
int Layout::indexOf(const Widget *widget) const {
    int result = -1;
    for(auto it : m_items) {
        ++result;
        if(it->m_attachedWidget == widget) {
            break;
        }
    }
    return result;
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
float Layout::spacing() const {
    return m_spacing;
}
/*!
    Sets the \a spacing between items in the layout.
*/
void Layout::setSpacing(float spacing) {
    m_spacing = spacing;
    invalidate();
}
/*!
    Sets the \a left, \a top, \a right and \a bottom margins for the layout.
*/
void Layout::setMargins(float left, float top, float right, float bottom) {
    m_margins = Vector4(left, top, right, bottom);
    invalidate();
}
/*!
    Returns the layout direction (Vertical or Horizontal).
*/
int Layout::direction() const {
    return m_direction;
}
/*!
    Sets the layout \a direction.
*/
void Layout::setDirection(int direction) {
    m_direction = direction;
    invalidate();
}
/*!
    Returns the size hint for the layout.
*/
Vector2 Layout::sizeHint() const {
    Vector2 result(m_margins.x, m_margins.y);
    for(auto it : m_items) {
        Vector2 size;
        if(it->m_attachedWidget) { // Item is widget
            if(it->m_attachedWidget->isVisible()) {
                RectTransform *rect = it->m_attachedWidget->rectTransform();
                size = rect->size();
            }
        } else { // Item is layout
            size = it->sizeHint();
        }

        if(m_direction == Vertical) {
            result.x = MAX(result.x, size.x);
            result.y += ((it != *m_items.begin()) ? m_spacing : 0.0f) + size.y;
        } else {
            result.x += ((it != *m_items.begin()) ? m_spacing : 0.0f) + size.x;
            result.y = MAX(result.y, size.y);
        }
    }
    result.x += m_margins.z;
    result.y += m_margins.w;

    return result;
}
/*!
    Marks the layout as dirty, indicating that it needs to be recomputed.
*/
void Layout::invalidate() {
    m_dirty = true;
}
/*!
    \internal
     Updates the layout. If the layout is marked as dirty, it recomputes the positions of child widgets and layouts.
*/
void Layout::update() {
    if(m_dirty) {
        float pos = ((m_direction == Vertical) ? m_margins.y : m_margins.x);

        for(auto it : m_items) {
            if(it->m_attachedWidget) {
                if(it->m_attachedWidget->isVisible()) {
                    RectTransform *rect = it->m_attachedWidget->rectTransform();
                    pos += (it != *m_items.begin()) ? m_spacing : 0.0f;

                    if(m_direction == Vertical) {
                        rect->setPosition(Vector3(m_margins.x + m_position.x, m_position.y - pos, 0.0f));
                        pos += rect->size().y;
                    } else {
                        rect->setPosition(Vector3(m_position.x + pos, m_position.y - m_margins.y, 0.0f));
                        pos += rect->size().x;
                    }
                }
            } else {
                it->m_position = (m_direction == Vertical) ? Vector2(0.0f, -pos) : Vector2(pos, 0.0f);
                it->update();

                Vector2 size(it->sizeHint());
                if(m_direction == Vertical) {
                    pos += size.y;
                } else {
                    pos += size.x;
                }
            }
        }
        m_dirty = false;
    }
}
