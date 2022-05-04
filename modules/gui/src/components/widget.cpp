#include "components/widget.h"

#include "components/recttransform.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>

#include <resources/pipeline.h>

#include <commandbuffer.h>

class WidgetPrivate {
public:
    WidgetPrivate() :
        m_pParent(nullptr),
        m_pTransform(nullptr) {

    }

    Widget *m_pParent;
    RectTransform *m_pTransform;
};

Widget::Widget() :
    p_ptr(new WidgetPrivate) {

}

Widget::~Widget() {
    if(p_ptr->m_pTransform) {
        p_ptr->m_pTransform->unsubscribe(this);
    }
    delete p_ptr;
    p_ptr = nullptr;
}

void Widget::update() {
    Renderable::update();
}

void Widget::draw(CommandBuffer &buffer, uint32_t layer) {
    if(p_ptr->m_pParent == nullptr && (layer == CommandBuffer::UI)) {
        if(p_ptr->m_pTransform) {
            p_ptr->m_pTransform->setSize(buffer.viewport());
        }
    }
}

AABBox Widget::bound() const {
    AABBox result;

    result.extent = (p_ptr->m_pTransform) ? Vector3(p_ptr->m_pTransform->size() * 0.5f, 0.0f) : Vector3();
    result.center = result.extent;

    return result * actor()->transform()->worldTransform();
}

void Widget::boundChanged() {

}

Widget *Widget::parentWidget() {
    return p_ptr->m_pParent;
}
/*!
    \internal
*/
void Widget::setRectTransform(RectTransform *transform) {
    if(p_ptr->m_pTransform) {
        p_ptr->m_pTransform->unsubscribe(this);
    }
    p_ptr->m_pTransform = transform;
    if(p_ptr->m_pTransform) {
        p_ptr->m_pTransform->subscribe(this);
    }
}

void Widget::setParent(Object *parent, int32_t position, bool force) {
    Renderable::setParent(parent, position, force);

    actorParentChanged();
}

void Widget::actorParentChanged() {
    Actor *object = actor();
    if(object) {
        setRectTransform(dynamic_cast<RectTransform *>(object->transform()));

        object = dynamic_cast<Actor *>(object->parent());
        if(object) {
            p_ptr->m_pParent = static_cast<Widget *>(object->component("Widget"));
        }
    }
}

void Widget::composeComponent() {
    Engine::objectCreate<RectTransform>("RectTransform", actor());
}

#ifdef SHARED_DEFINE
#include <viewport/handles.h>

bool Widget::drawHandles(ObjectList &selected) {
    if(isSelected(selected)) {
        AABBox box = bound();
        Handles::drawBox(box.center, Quaternion(), box.extent * 2.0f);
    }

    return false;
}
#endif
