#include "components/layout.h"

#include "components/recttransform.h"

/*!
    \class Layout
    \brief The Layout class is a base class for managing the layout and positioning of widgets within a graphical user interface.
    \inmodule Gui

    The Layout class is a base class used for managing the layout and positioning of widgets within a graphical user interface (GUI).
    It provides a structured way to organize UI elements, ensuring that they are placed efficiently and consistently on the screen.
    The Layout class is essential for developers who need to arrange multiple components (such as buttons, labels, text fields, etc.) in a clean and organized manner.
*/

Layout::Layout() :
        m_rectTransform(nullptr),
        m_spacing(0),
        m_orientation(Widget::Vertical),
        m_dirty(false) {

}

Layout::~Layout() {
    for(auto it : m_items) {
        it->m_attachedLayout = nullptr;
    }
}
/*!
    Adds a \a transform to the current layout.
*/
void Layout::addTransform(RectTransform *transform) {
    insertTransform(-1, transform);
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

        transform->m_attachedLayout = this;

        if(index >= 0 && index < m_items.size()) {
            m_items.insert(std::next(m_items.begin(), index), transform);
        } else {
            m_items.push_back(transform);
        }
        invalidate();

        // Tranfering the ownership
        transform->setParentTransform(rectTransform());
    }
}
/*!
    Removes a \a transform from the current layout.
*/
void Layout::removeTransform(RectTransform *transform) {
    if(transform) {
        m_items.remove(transform);
        transform->m_attachedLayout = nullptr;

        invalidate();
    }
}
/*!
    Returns the index of the specified \a transform.
*/
int Layout::indexOf(const RectTransform *transform) const {
    int result = -1;
    for(auto it : m_items) {
        ++result;
        if(it == transform) {
            return result;
        }
    }
    return -1;
}
/*!
    Returns transform located at \a index.
*/
RectTransform *Layout::transformAt(int index) {
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
RectTransform *Layout::rectTransform() {
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

    bool first = true;
    Vector2 result;
    for(auto it : m_items) {
        Vector2 size;

        float spacing = (!first) ? m_spacing : 0.0f;

        if(it->isEnabled()) {
            Vector4 margin(it->margin());

            size = Vector2(margin.w, margin.x);
            size += it->size();
            size += Vector2(margin.y, margin.z);

            first = false;
        }

        if(m_orientation == Widget::Vertical) {
            result.x = MAX(result.x, size.x);
            result.y += spacing + size.y;
        } else {
            result.x += spacing + size.x;
            result.y = MAX(result.y, size.y);
        }
    }

    result.x += padding.w + padding.y + border.y + border.w;
    result.y += padding.x + padding.z + border.x + border.z;

    return result;
}
/*!
    Marks the layout as dirty, indicating that it needs to be recomputed.
*/
void Layout::invalidate() {
    m_dirty = true;

    if(m_rectTransform) {
        Layout *attachedLayout = m_rectTransform->m_attachedLayout;
        if(attachedLayout) {
            attachedLayout->invalidate();
        }
    }
}
/*!
    \internal
    Updates the layout. If the layout is marked as dirty, it recomputes the positions of child widgets and layouts.
*/
void Layout::update() {
    if(m_dirty) {
        Vector4 padding(m_rectTransform->padding());
        Vector4 border(m_rectTransform->border());
        Vector2 size(m_rectTransform->size());
        Vector2 offset(padding.w + border.w, padding.x + border.x);

        // Change parrent size if needed
        Vector2 hint(sizeHint());
        if(m_rectTransform->horizontalPolicy() == RectTransform::Preferred) {
            size.x = hint.x;
        }
        if(m_rectTransform->verticalPolicy() == RectTransform::Preferred) {
            size.y = hint.y;
        }
        m_rectTransform->setSize(size);

        // Solve size
        Vector2 bottomRight(padding.y + border.y, padding.z + border.z);
        Vector2 internalSize(size - offset - bottomRight);

        solveItemsDimension(MAX(internalSize.x, 0.0f), true);
        solveItemsDimension(MAX(internalSize.y, 0.0f), false);

        // Solve positions
        solveItemsPosition(size.y, offset);

        m_dirty = false;
    }
}
/*!
    \internal
*/
void Layout::solveItemsDimension(int availableSpace, bool horizontal) {
    if(m_orientation == Widget::Vertical) {
        if(horizontal) {
            for(auto it : m_items) {
                if(it->horizontalPolicy() == RectTransform::Expanding) {
                    Vector4 margin(it->margin());
                    Vector2 size(it->size());
                    size.x = availableSpace - (margin.w + margin.y);
                    it->setSize(size);
                }
            }
            return;
        }
    } else {
        if(!horizontal) {
            for(auto it : m_items) {
                if(it->verticalPolicy() == RectTransform::Expanding) {
                    Vector4 margin(it->margin());
                    Vector2 size(it->size());
                    size.y = availableSpace - (margin.x + margin.z);
                    it->setSize(size);
                }
            }
            return;
        }
    }

    int totalPref = 0;
    int expandingCount = 0;
    int preferredCount = 0;
    int fixedCount = 0;

    for(auto it : m_items) {
        if(it->isEnabled()) {
            RectTransform::SizePolicy policy = horizontal ? it->horizontalPolicy() : it->verticalPolicy();
            int pref = horizontal ? it->size().x : it->size().y;
            Vector4 margin(it->margin());
            pref += horizontal ? (margin.w + margin.y) : (margin.x + margin.z);

            totalPref += pref;

            switch(policy) {
                case RectTransform::Fixed: fixedCount++; break;
                case RectTransform::Preferred: preferredCount++; break;
                case RectTransform::Expanding: expandingCount++; break;
                default: break;
            }
        }
    }

    int spacesCount = MAX(expandingCount + preferredCount + fixedCount - 1, 0);

    totalPref += spacesCount * m_spacing;

    if(availableSpace <= totalPref) { // Need to reduce size of widgets
        int remainingSpace = availableSpace;

        for(auto it : m_items) {
            if(it->isEnabled()) {
                RectTransform::SizePolicy policy = horizontal ? it->horizontalPolicy() : it->verticalPolicy();
                if(policy == RectTransform::Fixed) {
                    int pref = horizontal ? it->size().x : it->size().y;
                    Vector4 margin(it->margin());
                    pref += horizontal ? (margin.w + margin.y) : (margin.x + margin.z);

                    remainingSpace -= pref;
                }
            }
        }

        remainingSpace -= spacesCount * m_spacing;

        int flexibleCount = preferredCount + expandingCount;
        if(flexibleCount > 0 && remainingSpace > 0) {
            int spacePerFlexible = remainingSpace / flexibleCount;

            for(auto it : m_items) {
                if(it->isEnabled()) {
                    RectTransform::SizePolicy policy = horizontal ? it->horizontalPolicy() : it->verticalPolicy();
                    if(policy == RectTransform::Preferred || policy == RectTransform::Expanding) {
                        Vector4 margin(it->margin());
                        Vector2 size(it->size());
                        if(horizontal) {
                            size.x = spacePerFlexible - (margin.w + margin.y);
                        } else {
                            size.y = spacePerFlexible - (margin.x + margin.z);
                        }
                        it->setSize(size);
                    }
                }
            }
        }
    } else { // Need to expand widgets
        int extra = availableSpace - totalPref;

        for(auto it : m_items) {
            if(it->isEnabled()) {
                RectTransform::SizePolicy policy = horizontal ? it->horizontalPolicy() : it->verticalPolicy();

                float extraSize = 0;
                if(expandingCount > 0) {
                    if(policy == RectTransform::Expanding) {
                        extraSize = extra / static_cast<float>(expandingCount);
                    }
                } else if(preferredCount > 0) {
                    if(policy == RectTransform::Preferred) {
                        extraSize = extra / static_cast<float>(preferredCount);
                    }
                }

                if(extraSize != 0) {
                    Vector2 size(it->size());
                    if(horizontal) {
                        size.x += extraSize;
                    } else {
                        size.y += extraSize;
                    }
                    it->setSize(size);
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
        if(it->isEnabled()) {
            shift += (!first) ? m_spacing : 0.0f;

            Vector2 size(it->size());
            Vector2 pivot(it->pivot());
            Vector4 margin(it->margin());

            if(m_orientation == Widget::Vertical) {
                shift += size.y * pivot.y + margin.z;
                it->m_position.x = offset.x + size.x * pivot.x + margin.w;
                it->m_position.y = height - shift;
                it->setDirty();
                shift += size.y * (1.0f - pivot.y) + margin.x;
            } else {
                shift += size.x * pivot.x + margin.w;
                it->m_position.x = shift;
                it->m_position.y = offset.y - size.y * pivot.y - margin.z;
                it->setDirty();
                shift += size.x * (1.0f - pivot.x) + margin.y;
            }

            first = false;
        }
    }
}
