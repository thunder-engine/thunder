#include "components/recttransform.h"

#include "components/widget.h"
#include "components/actor.h"

class RectTransformPrivate {
public:
    RectTransformPrivate() :
        m_size(1.0f),
        m_pivot(0.5f),
        m_minAnchors(0.5f),
        m_maxAnchors(0.5f),
        m_parent(nullptr),
        m_dirty(true) {

    }

    void notify() {
        for(auto it : m_subscribers) {
            it->boundChanged();
        }
    }

    Matrix4 m_worldTransform;

    Vector2 m_size;
    Vector2 m_pivot;
    Vector2 m_minAnchors;
    Vector2 m_maxAnchors;
    list<Widget *> m_subscribers;

    RectTransform *m_parent;

    bool m_dirty;
};

RectTransform::RectTransform() :
    p_ptr(new RectTransformPrivate()) {

}

RectTransform::~RectTransform() {
    list<Widget *> list = p_ptr->m_subscribers;
    for(auto it : list) {
        it->setRectTransform(nullptr);
    }
    delete p_ptr;
    p_ptr = nullptr;
}

Vector2 RectTransform::size() const {
    return p_ptr->m_size;
}
void RectTransform::setSize(const Vector2 &size) {
    if(p_ptr->m_size != size) {
        Vector2 d = size - p_ptr->m_size;
        for(auto &it : children()) {
            RectTransform *child = dynamic_cast<RectTransform *>(it);
            if(child) {
                Vector2 minAnchors = child->minAnchors();
                child->setSize(child->size() + d * (child->maxAnchors() - minAnchors));
                child->setDirty();
            }
        }

        p_ptr->m_size = size;
        p_ptr->notify();
    }
}

Vector2 RectTransform::pivot() const {
    return p_ptr->m_pivot;
}
void RectTransform::setPivot(const Vector2 &pivot) {
    if(p_ptr->m_pivot != pivot) {
        p_ptr->m_pivot = pivot;
        p_ptr->notify();
    }
    setDirty();
}

Vector2 RectTransform::minAnchors() const {
    return p_ptr->m_minAnchors;
}
void RectTransform::setMinAnchors(const Vector2 &anchors) {
    p_ptr->m_minAnchors = anchors;
    setDirty();
}

Vector2 RectTransform::maxAnchors() const {
    return p_ptr->m_maxAnchors;
}
void RectTransform::setMaxAnchors(const Vector2 &anchors) {
    p_ptr->m_maxAnchors = anchors;
    setDirty();
}

bool RectTransform::isHovered(float x, float y) const {
    Actor *parent = actor();
    if(parent) {
        Vector3 pos = Vector3(p_ptr->m_worldTransform[12] + p_ptr->m_pivot.x,
                              p_ptr->m_worldTransform[13] + p_ptr->m_pivot.y, 0.0f);
        if(x > pos.x && x < pos.x + p_ptr->m_size.x &&
           y > pos.y && y < pos.y + p_ptr->m_size.y) {
            return true;
        }
    }
    return false;
}

void RectTransform::subscribe(Widget *widget) {
    p_ptr->m_subscribers.push_back(widget);
}

void RectTransform::unsubscribe(Widget *widget) {
    p_ptr->m_subscribers.remove(widget);
}

void RectTransform::setParentTransform(Transform *parent, bool force) {
    p_ptr->m_parent = dynamic_cast<RectTransform *>(parent);

    Transform::setParentTransform(parent, force);
}

Matrix4 &RectTransform::worldTransform() const {
    if(p_ptr->m_dirty) {
        p_ptr->m_worldTransform = Transform::worldTransform();
        cleanDirty();
    }
    return p_ptr->m_worldTransform;
}

void RectTransform::setDirty() {
    Transform::setDirty();
    p_ptr->m_dirty = true;
}

void RectTransform::cleanDirty() const {
    if(p_ptr->m_parent) {
        Vector2 v = p_ptr->m_parent->p_ptr->m_size * p_ptr->m_parent->p_ptr->m_minAnchors;
        v -= p_ptr->m_size * p_ptr->m_pivot;
        p_ptr->m_worldTransform.mat[12] += v.x;
        p_ptr->m_worldTransform.mat[13] += v.y;
        p_ptr->m_dirty = false;
    }
}
