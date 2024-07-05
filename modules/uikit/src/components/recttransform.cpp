#include "components/recttransform.h"

#include "components/widget.h"
#include "components/layout.h"

/*!
    \class RectTransform
    \brief The RectTransform class represents a transformation that specifies the position, size, and anchoring of a UI element.
    \inmodule Gui

    The ProgressBar class is designed to provide a graphical representation of progress with customizable appearance and range.
    It supports features such as setting the minimum and maximum values, adjusting the progress value, and specifying visual elements for background and progress indicator.
*/

RectTransform::RectTransform() :
        m_pivot(0.5f),
        m_minAnchors(0.5f),
        m_maxAnchors(0.5f),
        m_layout(nullptr),
        m_attachedLayout(nullptr) {

}

RectTransform::~RectTransform() {
    if(m_attachedLayout) {
        m_attachedLayout->removeTransform(this);
    }

    std::list<Widget *> list = m_subscribers;
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
        if(abs(m_minAnchors.x - m_maxAnchors.x) <= EPSILON) {
            m_size.x = size.x - m_margin.y - m_margin.w;
        }

        if(abs(m_minAnchors.y - m_maxAnchors.y) <= EPSILON) {
            m_size.y = size.y - m_margin.x - m_margin.z;
        }

        setDirty();
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

        setDirty();
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

        resetSize();
        setDirty();
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

        resetSize();
        setDirty();
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

    resetSize();
    setDirty();
}
/*!
    Returns the margin offsets of the RectTransform.
    The Vector4 contains offsets in top, right, bottom and left order.
*/
Vector4 RectTransform::margin() const {
    return m_margin;
}
/*!
    Sets the top, right, bottom and left \a margin offsets of the RectTransform.
*/
void RectTransform::setMargin(const Vector4 margin) {
    if(m_margin != margin) {
        m_margin = margin;

        setDirty();
    }
}
/*!
    Returns the border width of the RectTransform.
    The Vector4 contains border widths in top, right, bottom and left order.
*/
Vector4 RectTransform::border() const {
    return m_border;
}
/*!
    Sets the top, right, bottom and left \a border width of the RectTransform.
*/
void RectTransform::setBorder(const Vector4 border) {
    if(m_border != border) {
        m_border = border;

        notify();
    }
}
/*!
    Returns the padding offset of the RectTransform.
    The Vector4 contains padding offsets in top, right, bottom and left order.
*/
Vector4 RectTransform::padding() const {
    return m_padding;
}
/*!
    Sets the top, right, bottom and left \a padding offsets of the RectTransform.
*/
void RectTransform::setPadding(const Vector4 padding) {
    if(m_padding != padding) {
        m_padding = padding;
    }
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
    m_layout->setRectTransform(this);
}
/*!
    \internal
    Internal method that returns the axis-aligned bounding box (AABBox).
*/
AABBox RectTransform::bound() const {
    AABBox result;

    result.extent = Vector3(m_size * 0.5f, 0.0f);
    result.center = result.extent;

    return result * worldTransform();
}
/*!
    \internal
    Marks the RectTransform as dirty and triggers a recalculation.
*/
void RectTransform::setDirty() {
    m_dirty = true;

    recalcSize();
    notify();

    Transform::setDirty();
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
        Vector2 v2 = parentRect->m_size * m_minAnchors - m_bottomLeft;

        m_worldTransform[12] += (abs(m_minAnchors.x - m_maxAnchors.x) <= EPSILON) ? v1.x : v2.x;
        m_worldTransform[13] += (abs(m_minAnchors.y - m_maxAnchors.y) <= EPSILON) ? v1.y : v2.y;
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

    if(m_layout) {
        m_layout->invalidate();
    }
}
/*!
    \internal
    Recalculates the size of the RectTransform based on anchors and offsets.
*/
void RectTransform::recalcSize() {
    Vector2 parentSize;
    RectTransform *parentRect = dynamic_cast<RectTransform *>(m_parent);
    if(parentRect) {
        parentSize = parentRect->size();
    }

    if(abs(m_minAnchors.x - m_maxAnchors.x) <= EPSILON ) {
        m_bottomLeft.x = m_size.x * m_pivot.x;
        m_topRight.x = m_size.x * (1.0 - m_pivot.x);
    } else {
        m_size.x = m_topRight.x + parentSize.x * (m_maxAnchors.x - m_minAnchors.x) + m_bottomLeft.x;
    }

    if(abs(m_minAnchors.y - m_maxAnchors.y) <= EPSILON) {
        m_bottomLeft.y = m_size.y * m_pivot.y;
        m_topRight.y = m_size.y * (1.0 - m_pivot.y);
    } else {
        m_size.y = m_topRight.y + parentSize.y * (m_maxAnchors.y - m_minAnchors.y) + m_bottomLeft.y;
    }

    if(m_layout) {
        Vector2 hint(m_layout->sizeHint());

        m_size.x = MAX(hint.x, m_size.x);
        m_size.y = MAX(hint.y, m_size.y);
    }
}
/*!
    \internal
    Resets the size of the RectTransform based on anchors and offsets.
*/
void RectTransform::resetSize() {
    if(abs(m_minAnchors.x - m_maxAnchors.x) <= EPSILON) {
        m_size.x = m_bottomLeft.x + m_topRight.x;
    }

    if(abs(m_minAnchors.y - m_maxAnchors.y) <= EPSILON) {
        m_size.y = m_bottomLeft.y + m_topRight.y;
    }
}
