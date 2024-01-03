#include "components/gui/recttransform.h"

#include "components/gui/widget.h"
#include "components/gui/layout.h"
#include "components/actor.h"

/*!
    \class RectTransform
    \brief The RectTransform class represents a transformation that specifies the position, size, and anchoring of a UI element.
    \inmodule Gui

    The ProgressBar class is designed to provide a graphical representation of progress with customizable appearance and range.
    It supports features such as setting the minimum and maximum values, adjusting the progress value, and specifying visual elements for background and progress indicator.
*/

RectTransform::RectTransform() :
        m_bottomLeft(0.0f),
        m_topRight(0.0f),
        m_pivot(0.5f),
        m_minAnchors(0.5f),
        m_maxAnchors(0.5f),
        m_layout(nullptr) {

}

RectTransform::~RectTransform() {
    list<Widget *> list = m_subscribers;
    for(auto it : list) {
        it->setRectTransform(nullptr);
    }

    delete m_layout;
}
/*!
    Returns the size of the associated UI element.
*/
Vector2 RectTransform::size() const {
    return m_size;
}
/*!
    Sets the \a size of the RectTransform.
*/
void RectTransform::setSize(const Vector2 size) {
    Vector2 s = RectTransform::size();
    if(s != size) {
        Vector2 p;
        RectTransform *parentRect = dynamic_cast<RectTransform *>(m_parent);
        if(parentRect) {
            p = parentRect->size();
        }

        if(m_minAnchors.x == m_maxAnchors.x) {
            m_size.x = size.x;
            if(parentRect) {
                m_bottomLeft.x = (p.x - size.x) * m_pivot.x;
                m_topRight.x = (p.x - size.x) * (1.0 - m_pivot.x);
            } else {
                m_bottomLeft.x = size.x * m_pivot.x;
                m_topRight.x = size.x * (1.0 - m_pivot.x);
            }
        }

        if(m_minAnchors.y == m_maxAnchors.y) {
            m_size.y = size.y;
            if(parentRect) {
                m_bottomLeft.y = (p.y - size.y) * m_pivot.y;
                m_topRight.y = (p.y - size.y) * (1.0 - m_pivot.y);
            } else {
                m_bottomLeft.y = size.y * m_pivot.y;
                m_topRight.y = size.y * (1.0 - m_pivot.y);
            }
        }
        setDirty(true);
    }
}
/*!
    Returns the pivot point of the RectTransform.
*/
Vector2 RectTransform::pivot() const {
    return m_pivot;
}
/*!
    Sets the \a pivot point of the RectTransform.
*/
void RectTransform::setPivot(const Vector2 pivot) {
    if(m_pivot != pivot) {
        m_pivot = pivot;
        setDirty(true);
    }
}
/*!
    Returns the minimum anchors of the RectTransform.
*/
Vector2 RectTransform::minAnchors() const {
    return m_minAnchors;
}
/*!
    Sets the minimum \a anchors of the RectTransform.
*/
void RectTransform::setMinAnchors(const Vector2 anchors) {
    if(m_minAnchors != anchors) {
        m_minAnchors = anchors;
        setDirty(true);
    }
}
/*!
    Returns the maximum anchors of the RectTransform.
*/
Vector2 RectTransform::maxAnchors() const {
    return m_maxAnchors;
}
/*!
    Sets the maximum \a anchors of the RectTransform.
*/
void RectTransform::setMaxAnchors(const Vector2 anchors) {
    if(m_maxAnchors != anchors) {
        m_maxAnchors = anchors;
        setDirty(true);
    }
}
/*!
    Sets both the \a minimum and \a maximum anchors of the RectTransform.
*/
void RectTransform::setAnchors(const Vector2 minimum, const Vector2 maximum) {
    if(m_minAnchors != minimum) {
        m_minAnchors = minimum;
    }

    if(m_maxAnchors != maximum) {
        m_maxAnchors = maximum;
    }

    setDirty(true);
}
/*!
    Returns the bottom-left offset of the RectTransform.
*/
Vector2 RectTransform::offsetMin() const {
    return m_bottomLeft;
}
/*!
    Sets the bottom-left \a offset of the RectTransform.
*/
void RectTransform::setOffsetMin(const Vector2 offset) {
    if(m_bottomLeft != offset) {
        m_bottomLeft = offset;
        setDirty(true);
    }
}
/*!
    Returns the top-right offset of the RectTransform.
*/
Vector2 RectTransform::offsetMax() const {
    return m_topRight;
}
/*!
    Sets the top-right \a offset of the RectTransform.
*/
void RectTransform::setOffsetMax(const Vector2 offset) {
    if(m_topRight != offset) {
        m_topRight = offset;
        setDirty(true);
    }
}
/*!
    Sets both the \a minimum and \a maximum offsets of the RectTransform.
*/
void RectTransform::setOffsets(const Vector2 minimum, const Vector2 maximum) {
    if(m_bottomLeft != minimum) {
        m_bottomLeft = minimum;
    }

    if(m_topRight != maximum) {
        m_topRight = maximum;
    }

    setDirty(true);
}
/*!
    Returns true if the point with coodinates \a x and \a y is within the bounds, otherwise false.
*/
bool RectTransform::isHovered(float x, float y) const {
    Vector2 pos(m_worldTransform[12],
                m_worldTransform[13]);

    if(x > pos.x && x < pos.x + m_size.x &&
       y > pos.y && y < pos.y + m_size.y) {
        return true;
    }

    return false;
}
/*!
    Subscribes a \a widget to changes in the RectTransform.
*/
void RectTransform::subscribe(Widget *widget) {
    m_subscribers.push_back(widget);
}
/*!
    Unsubscribes a \a widget from changes in the RectTransform.
*/
void RectTransform::unsubscribe(Widget *widget) {
    m_subscribers.remove(widget);
}
/*!
    Returns the layout assigned to the RectTransform.
*/
Layout *RectTransform::layout() const {
    return m_layout;
}
/*!
    Sets the \a layout for the RectTransform.
*/
void RectTransform::setLayout(Layout *layout) {
    m_layout = layout;
}
/*!
    Returns the world transformation matrix of the RectTransform.
*/
Matrix4 RectTransform::worldTransform() const {
    if(m_dirty) {
        m_worldTransform = Transform::worldTransform();
        cleanDirty();
    }

    return m_worldTransform;
}
/*!
    \internal
    Marks the RectTransform as \a dirty and triggers a recalculation.
*/
void RectTransform::setDirty(bool dirty) {
    m_dirty = true;

    recalcSize();
    notify();

    Transform::setDirty(dirty);
}
/*!
    \internal
    Cleans the dirty state of the RectTransform and updates the world transformation matrix.
*/
void RectTransform::cleanDirty() const {
    Transform::cleanDirty();

    RectTransform *parentRect = dynamic_cast<RectTransform *>(m_parent);
    if(parentRect) {
        Vector2 parentCenter = parentRect->m_size * (m_minAnchors + m_maxAnchors) * 0.5f;
        Vector2 rectCenter = m_size * m_pivot;

        Vector2 v1 = parentCenter - rectCenter;
        Vector2 v2 = parentRect->m_size * m_minAnchors + m_bottomLeft;

        m_worldTransform[12] += (m_minAnchors.x == m_maxAnchors.x) ? v1.x : v2.x;
        m_worldTransform[13] += (m_minAnchors.y == m_maxAnchors.y) ? v1.y : v2.y;
    }
}
/*!
    \internal
    Notifies subscribers (widgets) of a change in the RectTransform.
*/
void RectTransform::notify() {
    for(auto it : m_subscribers) {
        it->boundChanged(size());
    }
}
/*!
    \internal
    Recalculates the size of the RectTransform based on anchors and offsets.
*/
void RectTransform::recalcSize() {
    RectTransform *parentRect = dynamic_cast<RectTransform *>(m_parent);
    if(parentRect) {
        Vector2 s = parentRect->size();
        if(m_maxAnchors.x != m_minAnchors.x) {
            m_size.x = s.x * (m_maxAnchors.x - m_minAnchors.x) - (m_topRight.x + m_bottomLeft.x);
        }

        if(m_maxAnchors.y != m_minAnchors.y) {
            m_size.y = s.y * (m_maxAnchors.y - m_minAnchors.y) - (m_topRight.y + m_bottomLeft.y);
        }
    } else {
        m_size = m_topRight + m_bottomLeft;
    }
}
