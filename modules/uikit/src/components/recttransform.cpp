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
        m_attachedLayout(nullptr),
        m_verticalPolicy(Maximum),
        m_horizontalPolicy(Maximum),
        m_mouseTracking(true) {

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
    if(m_size != size) {
        if(abs(m_minAnchors.x - m_maxAnchors.x) <= EPSILON) {
            m_size.x = size.x - m_margin.y - m_margin.w;
        }

        if(abs(m_minAnchors.y - m_maxAnchors.y) <= EPSILON) {
            m_size.y = size.y - m_margin.x - m_margin.z;
        }

        setDirty();
        recalcChilds();
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
        recalcChilds();
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

        setDirty();
        recalcChilds();
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

        setDirty();
        recalcChilds();
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

    setDirty();
    recalcChilds();
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
        recalcChilds();
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

        setDirty();
        recalcChilds();
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

        setDirty();
        recalcChilds();
    }
}
/*!
    Returns true if this area is interactable with mouse; otherwise returns false.
    Returns true by the default.
*/
bool RectTransform::mouseTracking() const {
    return m_mouseTracking;
}
/*!
    Sets mouse \a tracking enabled or disabled.
*/
void RectTransform::setMouseTracking(bool tracking) {
    m_mouseTracking = tracking;
}
/*!
    Returns true if the point with coordinates \a x and \a y is within the bounds, otherwise false.
*/
bool RectTransform::isHovered(float x, float y) const {
    if(!m_mouseTracking) {
        return false;
    }

    Vector2 pos(m_worldTransform[12],
                m_worldTransform[13]);

    if(x > pos.x && x < pos.x + m_size.x * m_worldScale.x &&
       y > pos.y && y < pos.y + m_size.y * m_worldScale.y) {
        return true;
    }

    return false;
}
/*!
    Returns the most top RectTransform in hierarchy wich contains the point with coodinates \a x and \a y.
    Returns null if no bounds.
*/
RectTransform *RectTransform::hoveredTransform(float x, float y) {
    if(isHovered(x, y)) {
        for(auto it : m_children) {
            RectTransform *rect = dynamic_cast<RectTransform *>(it);
            if(rect) {
                RectTransform *result = rect->hoveredTransform(x, y);
                if(result) {
                    return result;
                }
            }
        }
        return this;
    }

    return nullptr;
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
    if(m_layout) {
        m_layout->setRectTransform(this);
    }
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
    Sets current state of RectTransform to \a enabled or disabled.
*/
void RectTransform::setEnabled(bool enabled) {
    RectTransform *rect = dynamic_cast<RectTransform*>(m_parent);
    if(rect) {
        rect->recalcParent();
    }
}

RectTransform::SizePolicy RectTransform::verticalPolicy() const {
    return m_verticalPolicy;
}

void RectTransform::setVerticalPolicy(SizePolicy policy) {
    m_verticalPolicy = policy;
}

RectTransform::SizePolicy RectTransform::horizontalPolicy() const {
    return m_horizontalPolicy;
}

void RectTransform::setHorizontalPolicy(SizePolicy policy) {
    m_horizontalPolicy = policy;
}
/*!
    \internal
    Cleans the dirty state of the RectTransform and updates the world transformation matrix.
*/
void RectTransform::cleanDirty() const {
    if(m_dirty) {
        Transform::cleanDirty();

        RectTransform *parentRect = dynamic_cast<RectTransform *>(m_parent);
        if(parentRect) {
            Vector2 anchors(m_minAnchors + m_maxAnchors);
            Vector2 parentCenter(anchors * parentRect->m_size * 0.5f);

            float x;
            if(abs(m_minAnchors.x - m_maxAnchors.x) > EPSILON) { // fit to parent
                x = parentRect->m_size.x * m_minAnchors.x + m_margin.w;
            } else {
                x = parentCenter.x / parentRect->m_scale.x - m_size.x * m_pivot.x + m_margin.w;
            }

            float y;
            if(abs(m_minAnchors.y - m_maxAnchors.y) > EPSILON) { // fit to parent
                y = parentRect->m_size.y * m_minAnchors.y + m_margin.z;
            } else {
                y = parentCenter.y / parentRect->m_scale.y - m_size.y * m_pivot.y + m_margin.z;
            }

            m_transform[12] += x;
            m_transform[13] += y;

            Mathf::hashCombine(m_hash, x);
            Mathf::hashCombine(m_hash, y);

            m_worldTransform = parentRect->worldTransform() * m_transform;
        }
    }
}

float policyHelper(RectTransform::SizePolicy policy, float current, float hint) {
    switch(policy) {
        case RectTransform::Minimum: return MIN(current, hint);
        case RectTransform::Maximum: return MAX(current, hint);
        case RectTransform::Preferred: return hint;
        default: break;
    }

    return current;
}
/*!
    \internal
    Recalculates the size of the RectTransform based on anchors and offsets.
*/
void RectTransform::recalcChilds() const {
    Vector2 parentSize;
    RectTransform *parentRect = dynamic_cast<RectTransform *>(m_parent);
    if(parentRect) {
        parentSize = parentRect->size();

        Vector4 padding(parentRect->padding());
        Vector4 border(parentRect->border());
        parentSize -= Vector2(padding.y + padding.w, padding.x + padding.z);
        parentSize -= Vector2(border.y + border.w, border.x + border.z);
    }

    if(abs(m_minAnchors.x - m_maxAnchors.x) > EPSILON) { // fit to parent
        m_size.x = parentSize.x * (m_maxAnchors.x - m_minAnchors.x) - (m_margin.y + m_margin.w);
    }

    if(abs(m_minAnchors.y - m_maxAnchors.y) > EPSILON) { // fit to parent
        m_size.y = parentSize.y * (m_maxAnchors.y - m_minAnchors.y) - (m_margin.x + m_margin.z);
    }

    for(auto it : m_children) {
        RectTransform *rect = dynamic_cast<RectTransform *>(it);
        if(rect) {
            rect->recalcChilds();
        }
    }

    if(m_layout &&
            (m_horizontalPolicy != RectTransform::Fixed || m_verticalPolicy != RectTransform::Fixed)) {
        Vector2 hint(m_layout->sizeHint());

        m_size.x = policyHelper(m_horizontalPolicy, m_size.x, hint.x);
        m_size.y = policyHelper(m_verticalPolicy, m_size.y, hint.y);
    }

    // notify
    for(auto it : m_subscribers) {
        it->boundChanged(m_size);
    }

    if(m_layout) {
        m_layout->invalidate();
        m_layout->update();
    }

    m_dirty = true;
}

Vector2 RectTransform::sizeHint() const {
    if(m_layout) {
        return m_layout->sizeHint();
    }

    return Vector2();
}

Vector4 RectTransform::scissorArea() const {
    cleanDirty();
    return Vector4(m_worldTransform[12] + m_padding.w,
                   m_worldTransform[13] + m_padding.x,
                   m_size.x * m_worldScale.x - (m_padding.y + m_padding.w),
                   m_size.y * m_worldScale.y - (m_padding.x + m_padding.z));
}

void RectTransform::recalcParent() {
    if(m_layout) {
        Vector2 oldSize(m_size);

        bool isBreak = true;

        if(m_verticalPolicy == Preferred || m_horizontalPolicy == Preferred) {
            Vector2 hint(m_layout->sizeHint());
            if(m_verticalPolicy == Preferred) {
                m_size.y = hint.y;
                isBreak = false;
            }

            if(m_horizontalPolicy == Preferred) {
                m_size.x = hint.x;
                isBreak = false;
            }
        }

        if(isBreak) {
            return;
        }

        if(oldSize != m_size) {
            setDirty();

            for(auto it : m_subscribers) {
                it->boundChanged(m_size);
            }

            if(m_layout) {
                m_layout->invalidate();
                m_layout->update();
            }

            RectTransform *parentRect = dynamic_cast<RectTransform *>(m_parent);
            if(parentRect) {
                parentRect->recalcParent();
            }
        }
    }
}
