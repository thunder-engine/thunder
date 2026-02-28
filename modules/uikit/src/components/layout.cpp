#include "components/layout.h"

#include "components/recttransform.h"

#include <components/actor.h>

/*!
    \class Layout
    \brief The Layout class is a base class for managing the layout and positioning of widgets within a graphical user interface.
    \inmodule Gui

    The Layout class is a base class used for managing the layout and positioning of widgets within a graphical user interface (GUI).
    It provides a structured way to organize UI elements, ensuring that they are placed efficiently and consistently on the screen.
    The Layout class is essential for developers who need to arrange multiple components (such as buttons, labels, text fields, etc.) in a clean and organized manner.
*/

Layout::Layout() :
        m_parentLayout(nullptr),
        m_attachedTransform(nullptr),
        m_rectTransform(nullptr),
        m_spacing(0),
        m_orientation(Widget::Vertical),
        m_dirty(false) {

}

Layout::~Layout() {
    for(auto it : m_items) {
        if(it->m_attachedTransform) {
            it->m_attachedTransform->m_attachedLayout = nullptr;
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
    Adds a \a transform to the current layout.
*/
void Layout::addTransform(RectTransform *transform) {
    insertTransform(-1, transform);
}
/*!
    Inserts a child \a layout at the specified \a index.
    If -1, the layout is appended to the end.
*/
void Layout::insertLayout(int index, Layout *layout) {
    if(layout) {
        layout->m_parentLayout = this;
        if(index > 0 && index < m_items.size()) {
            m_items.insert(std::next(m_items.begin(), index), layout);
        } else {
            m_items.push_back(layout);
        }
        invalidate();

        if(m_rectTransform) {
            m_rectTransform->recalcParent();
        }

        update();
    }
}
/*!
     Inserts a \a transform at the specified \a index.
     If -1, the layout is appended to the end.
*/
void Layout::insertTransform(int index, RectTransform *transform) {
    if(transform) {
        if(transform->m_attachedLayout) {
            transform->m_attachedLayout->removeTransform(transform);
            transform->m_attachedLayout = nullptr;
        }

        Layout *layout = new Layout;
        layout->m_attachedTransform = transform;
        layout->m_attachedTransform->setAnchors(Vector2(0.0f, 1.0f), Vector2(0.0f, 1.0f));
        transform->m_attachedLayout = this;

        insertLayout(index, layout);

        // Tranfering the ownership
        RectTransform *rect = rectTransform();
        transform->actor()->setParent(rect->actor(), index);
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
    Removes a \a transform from the current layout.
*/
void Layout::removeTransform(RectTransform *transform) {
    if(transform) {
        for(auto it : m_items) {
            if(it->m_attachedTransform == transform) {
                Layout *tmp = it;
                m_items.remove(tmp);
                delete tmp;

                transform->m_attachedLayout = nullptr;

                invalidate();

                if(m_rectTransform) {
                    m_rectTransform->recalcParent();
                }

                break;
            }
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
    Returns the index of the specified \a transform.
*/
int Layout::indexOf(const RectTransform *transform) const {
    int result = -1;
    for(auto it : m_items) {
        ++result;
        if(it->m_attachedTransform == transform) {
            break;
        }
    }
    return result;
}
/*!
    Returns transform located at \a index.
*/
RectTransform *Layout::transformAt(int index) {
    if(index > -1 && index < m_items.size()) {
        auto it = std::next(m_items.begin(), index);
        return (*it)->m_attachedTransform;
    }
    return nullptr;
}
/*!
    Returns the parent rect transform of this layout, or nullptr if this layout is not installed on any rect transform.
    If the layout is a sub-layout, this function returns the parent rect transform of the parent layout.
*/
RectTransform *Layout::rectTransform() {
    if(m_parentLayout) {
        return m_parentLayout->rectTransform();
    }

    return m_rectTransform;
}
/*!
    \internal
    Sets the parent rect \a tranform for this layout.
    It will be used to notify parent rect transform for any layout changes.
*/
void Layout::setRectTransform(RectTransform *transform) {
    m_rectTransform = transform;
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
    Vector4 padding;
    Vector4 border;
    if(m_rectTransform) {
        padding = m_rectTransform->padding();
        border = m_rectTransform->border();
    }

    Vector2 result(padding.w + border.w, padding.x + border.x);
    for(auto it : m_items) {
        Vector2 size;

        if(it->m_attachedTransform) { // Item is widget
            if(it->m_attachedTransform->actor()->isEnabled()) {
                Vector4 margin = it->m_attachedTransform->margin();

                size = Vector2(margin.w, margin.x);
                size += it->m_attachedTransform->size();
                size += Vector2(margin.y, margin.z);
            }
        } else { // Item is layout
            size = it->sizeHint();
        }

        if(m_orientation == Widget::Vertical) {
            result.x = MAX(result.x, size.x);
            result.y += ((it != *m_items.begin()) ? m_spacing : 0.0f) + size.y;
        } else {
            result.x += ((it != *m_items.begin()) ? m_spacing : 0.0f) + size.x;
            result.y = MAX(result.y, size.y);
        }
    }

    result.x += padding.y + border.y;
    result.y += padding.z + border.z;

    return result;
}
/*!
    Marks the layout as dirty, indicating that it needs to be recomputed.
*/
void Layout::invalidate() {
    m_dirty = true;
    for(auto it : m_items) {
        if(it->m_attachedTransform == nullptr) {
            it->invalidate();
        }
    }
}
/*!
    \internal
     Updates the layout. If the layout is marked as dirty, it recomputes the positions of child widgets and layouts.
*/
void Layout::update() {
    if(m_dirty) {
        Vector4 padding;
        Vector4 border;
        if(m_rectTransform) {
            padding = m_rectTransform->padding();
            border = m_rectTransform->border();
        }

        // Top and left paddings
        float offset = ((m_orientation == Widget::Vertical) ? padding.x + border.x : padding.w + border.w);

        for(auto it : m_items) {
            if(it->m_attachedTransform) {
                RectTransform *r = it->m_attachedTransform;
                if(r->actor()->isEnabled()) {
                    offset += (it != *m_items.begin()) ? m_spacing : 0.0f;

                    Vector2 size(r->size());
                    Vector2 pivot(r->pivot());
                    Vector4 margin(r->margin());

                    if(m_orientation == Widget::Vertical) {
                        offset += size.y * (1.0f - pivot.y) - margin.z;
                        Vector3 newPos(m_position.x + padding.w + border.w + size.x * pivot.x,
                                       m_position.y - offset, 0.0f);

                        r->setPosition(newPos);
                        offset += size.y * pivot.y;
                    } else {
                        offset += size.x * pivot.x + margin.w;
                        Vector3 newPos(m_position.x + offset,
                                       m_position.y - padding.x - border.x - size.y * pivot.y, 0.0f);

                        r->setPosition(newPos);
                        offset += size.x * (1.0f - pivot.x);
                    }
                }
            } else {
                offset += (it != *m_items.begin()) ? m_spacing : 0.0f;

                it->m_position = (m_orientation == Widget::Vertical) ? Vector2(m_position.x, m_position.y - offset) :
                                                                       Vector2(m_position.x + offset, m_position.y);
                it->update();

                Vector2 size(it->sizeHint());
                if(m_orientation == Widget::Vertical) {
                    offset += size.y;
                } else {
                    offset += size.x;
                }
            }
        }

        m_dirty = false;
    }
}
