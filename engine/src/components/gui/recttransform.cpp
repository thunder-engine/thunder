#include "components/gui/recttransform.h"

#include "components/gui/widget.h"
#include "components/gui/layout.h"
#include "components/actor.h"

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

Vector2 RectTransform::size() const {
    return m_size;
}
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
        setDirty();
    }
}

Vector2 RectTransform::pivot() const {
    return m_pivot;
}
void RectTransform::setPivot(const Vector2 pivot) {
    if(m_pivot != pivot) {
        m_pivot = pivot;
        setDirty();
    }
}

Vector2 RectTransform::minAnchors() const {
    return m_minAnchors;
}
void RectTransform::setMinAnchors(const Vector2 anchors) {
    if(m_minAnchors != anchors) {
        m_minAnchors = anchors;
        setDirty();
    }
}

Vector2 RectTransform::maxAnchors() const {
    return m_maxAnchors;
}
void RectTransform::setMaxAnchors(const Vector2 anchors) {
    if(m_maxAnchors != anchors) {
        m_maxAnchors = anchors;
        setDirty();
    }
}

void RectTransform::setAnchors(const Vector2 min, const Vector2 max) {
    if(m_minAnchors != min) {
        m_minAnchors = min;
    }

    if(m_maxAnchors != max) {
        m_maxAnchors = max;
    }

    setDirty();
}

Vector2 RectTransform::offsetMin() const {
    return m_bottomLeft;
}
void RectTransform::setOffsetMin(const Vector2 offset) {
    if(m_bottomLeft != offset) {
        m_bottomLeft = offset;
        setDirty();
    }
}

Vector2 RectTransform::offsetMax() const {
    return m_topRight;
}
void RectTransform::setOffsetMax(const Vector2 offset) {
    if(m_topRight != offset) {
        m_topRight = offset;
        setDirty();
    }
}

void RectTransform::setOffsets(const Vector2 min, const Vector2 max) {
    if(m_bottomLeft != min) {
        m_bottomLeft = min;
    }

    if(m_topRight != max) {
        m_topRight = max;
    }

    setDirty();
}

bool RectTransform::isHovered(float x, float y) const {
    Vector2 pos(m_worldTransform[12],
                m_worldTransform[13]);

    if(x > pos.x && x < pos.x + m_size.x &&
       y > pos.y && y < pos.y + m_size.y) {
        return true;
    }

    return false;
}

void RectTransform::subscribe(Widget *widget) {
    m_subscribers.push_back(widget);
}

void RectTransform::unsubscribe(Widget *widget) {
    m_subscribers.remove(widget);
}

Layout *RectTransform::layout() const {
    return m_layout;
}
void RectTransform::setLayout(Layout *layout) {
    m_layout = layout;
}

Matrix4 RectTransform::worldTransform() const {
    if(m_dirty) {
        m_worldTransform = Transform::worldTransform();
        cleanDirty();
    }

    return m_worldTransform;
}

void RectTransform::setDirty() {
    m_dirty = true;

    recalcSize();
    notify();

    Transform::setDirty();
}

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

void RectTransform::notify() {
    for(auto it : m_subscribers) {
        it->boundChanged(size());
    }
}

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
