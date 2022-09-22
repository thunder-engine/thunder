#include "components/widget.h"

#include "components/recttransform.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <commandbuffer.h>

Widget *Widget::m_focusWidget = nullptr;

Widget::Widget() :
    m_parent(nullptr),
    m_transform(nullptr) {

}

Widget::~Widget() {
    if(m_transform) {
        m_transform->unsubscribe(this);
    }
}
/*!
    \internal
*/
void Widget::update() {
    Renderable::update();
}
/*!
    \internal
*/
void Widget::draw(CommandBuffer &buffer, uint32_t layer) {
    if(m_parent == nullptr && (layer == CommandBuffer::UI)) {
        if(m_transform) {
            m_transform->setSize(buffer.viewport());
        }
    }
}
/*!
    \internal
*/
AABBox Widget::bound() const {
    AABBox result;

    result.extent = (m_transform) ? Vector3(m_transform->size() * 0.5f, 0.0f) : Vector3();
    result.center = result.extent;

    return result * actor()->transform()->worldTransform();
}

void Widget::boundChanged(const Vector2 &size) {
    A_UNUSED(size);
}

Widget *Widget::parentWidget() {
    return m_parent;
}

RectTransform *Widget::rectTransform() const {
    return m_transform;
}

Widget *Widget::focusWidget() {
    return m_focusWidget;
}
/*!
    \internal
*/
void Widget::setFocusWidget(Widget *widget) {
    m_focusWidget = widget;
}
/*!
    \internal
*/
void Widget::setRectTransform(RectTransform *transform) {
    if(m_transform) {
        m_transform->unsubscribe(this);
    }
    m_transform = transform;
    if(m_transform) {
        m_transform->subscribe(this);
    }
}

void Widget::setParent(Object *parent, int32_t position, bool force) {
    Renderable::setParent(parent, position, force);

    actorParentChanged();
}
/*!
    \internal
*/
void Widget::actorParentChanged() {
    Actor *object = actor();
    if(object) {
        setRectTransform(dynamic_cast<RectTransform *>(object->transform()));

        object = dynamic_cast<Actor *>(object->parent());
        if(object) {
            m_parent = static_cast<Widget *>(object->component("Widget"));
        }
    }
}

void Widget::composeComponent() {
    setRectTransform(Engine::objectCreate<RectTransform>("RectTransform", actor()));
}

#ifdef SHARED_DEFINE
#include <viewport/handles.h>

bool Widget::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        AABBox box = bound();
        Handles::s_Color = Vector4(0.5f, 1.0f, 0.5f, 1.0f);
        Handles::drawBox(box.center, Quaternion(), box.extent * 2.0f);
        Handles::s_Color = Handles::s_Normal;
    }
    return false;
}
#endif
