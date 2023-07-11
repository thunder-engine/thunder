#include "components/gui/layout.h"

#include "components/gui/recttransform.h"
#include "components/actor.h"

Layout::Layout() :
        m_parentTransform(nullptr),
        m_spacing(0.0f),
        m_direction(Vertical),
        m_dirty(false) {

}

Layout::~Layout() {
    for(auto it : m_widgets) {
        it->m_attachedLayout = nullptr;
    }
}

void Layout::addTransform(RectTransform *transform) {
    m_widgets.push_back(transform);
    transform->m_attachedLayout = this;
    invalidate();
}
void Layout::removeTransform(RectTransform *transform) {
    m_widgets.remove(transform);
    invalidate();
}

int Layout::indexOf(const Widget *widget) const {
    int result = -1;
    for(auto it : m_widgets) {
        ++result;
        if(it == widget->rectTransform()) {
            break;
        }
    }
    return result;
}

float Layout::spacing() const {
    return m_spacing;
}
void Layout::setSpacing(float spacing) {
    m_spacing = spacing;
    invalidate();
}

void Layout::setMargins(float left, float top, float right, float bottom) {
    m_margins = Vector4(left, top, right, bottom);
    invalidate();
}

int Layout::direction() const {
    return m_direction;
}
void Layout::setDirection(int direction) {
    m_direction = direction;
    invalidate();
}

Vector2 Layout::sizeHint() const {
    Vector2 result(m_margins.x, m_margins.y);
    for(auto it : m_widgets) {
        if(it->actor()->isEnabled()) {
            Vector2 size(it->size());
            if(m_direction == Vertical) {
                result.x = MAX(result.x, size.x);
                result.y += ((it != *m_widgets.begin()) ? m_spacing : 0.0f) + size.y;
            } else {
                result.x += ((it != *m_widgets.begin()) ? m_spacing : 0.0f) + size.x;
                result.y = MAX(result.y, size.y);
            }
        }
    }
    result.x += m_margins.z;
    result.y += m_margins.w;

    return result;
}

void Layout::invalidate() {
    m_dirty = true;
}

void Layout::update() {
    if(m_dirty) {
        float pos = (m_direction == Vertical) ? m_margins.y : m_margins.x;
        for(auto it : m_widgets) {
            if(it->actor()->isEnabled()) {
                pos += (it != *m_widgets.begin()) ? m_spacing : 0.0f;
                if(m_direction == Vertical) {
                    it->setPosition(Vector3(m_margins.x, -pos, 0.0f));
                    pos += it->size().y;
                } else {
                    it->setPosition(Vector3(pos, -m_margins.y, 0.0f));
                    pos += it->size().x;
                }
            }
        }
        m_dirty = false;
    }
}
