#include "components/recttransform.h"

#include "components/widget.h"

class RectTransformPrivate {
public:
    RectTransformPrivate() :
        m_Size(1.0f),
        m_Pivot(0.0f),
        m_minAnchors(0.5f),
        m_maxAnchors(0.5f) {

    }

    void notify() {
        for(auto it : m_Subscribers) {
            it->boundChanged();
        }
    }

    Vector2 m_Size;
    Vector2 m_Pivot;
    Vector2 m_minAnchors;
    Vector2 m_maxAnchors;
    list<Widget *> m_Subscribers;
};

RectTransform::RectTransform() :
    p_ptr(new RectTransformPrivate) {

}

RectTransform::~RectTransform() {
    list<Widget *> list = p_ptr->m_Subscribers;
    for(auto it : list) {
        it->setRectTransform(nullptr);
    }
    delete p_ptr;
}

Vector2 RectTransform::size() const {
    return p_ptr->m_Size;
}
void RectTransform::setSize(const Vector2 &size) {
    if(p_ptr->m_Size != size) {
        Vector2 d = size - p_ptr->m_Size;
        for(auto &it : children()) {
            RectTransform *child = dynamic_cast<RectTransform *>(it);
            if(child) {
                Vector2 minAnchors = child->minAnchors();
                child->setSize(child->size() + d * (child->maxAnchors() - minAnchors));
                child->setPosition(child->position() + Vector3(d * minAnchors, 0.0f));
            }
        }

        p_ptr->m_Size = size;
        p_ptr->notify();
    }
}

Vector2 RectTransform::pivot() const {
    return p_ptr->m_Pivot;
}
void RectTransform::setPivot(const Vector2 &pivot) {
    if(p_ptr->m_Pivot != pivot) {
        p_ptr->m_Pivot = pivot;
        p_ptr->notify();
    }
}

Vector2 RectTransform::minAnchors() const {
    return p_ptr->m_minAnchors;
}
void RectTransform::setMinAnchors(const Vector2 &anchors) {
    p_ptr->m_minAnchors = anchors;
}

Vector2 RectTransform::maxAnchors() const {
    return p_ptr->m_maxAnchors;
}
void RectTransform::setMaxAnchors(const Vector2 &anchors) {
    p_ptr->m_maxAnchors = anchors;
}

bool RectTransform::isHovered(float x, float y) const {
    Actor *parent = actor();
    if(parent) {
        Vector3 pos = worldPosition() + Vector3(p_ptr->m_Pivot, 0.0f);
        if(x > pos.x && x < pos.x + p_ptr->m_Size.x &&
           y > pos.y && y < pos.y + p_ptr->m_Size.y) {
            return true;
        }
    }
    return false;
}

void RectTransform::subscribe(Widget *widget) {
    p_ptr->m_Subscribers.push_back(widget);
}

void RectTransform::unsubscribe(Widget *widget) {
    p_ptr->m_Subscribers.remove(widget);
}
