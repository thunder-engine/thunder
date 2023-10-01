#include "components/gui/widget.h"

#include "components/gui/recttransform.h"
#include "components/gui/layout.h"

#include "components/actor.h"

#include "systems/rendersystem.h"

#include "commandbuffer.h"
#include "gizmos.h"

Widget *Widget::m_focusWidget = nullptr;

/*!
    \class Widget
    \brief The Widget class is the base class of all user interface objects.
    \inmodule Engine
*/

Widget::Widget() :
    m_parent(nullptr),
    m_transform(nullptr),
    m_attachedLayout(nullptr)  {

}

Widget::~Widget() {
    if(m_attachedLayout) {
        m_attachedLayout->removeWidget(this);
    }
    if(m_transform) {
        m_transform->unsubscribe(this);
    }
    static_cast<RenderSystem *>(system())->removeWidget(this);
}
/*!
    \internal
*/
void Widget::update() {
    NativeBehaviour::update();

    if(m_transform) {
        Layout *layout = m_transform->layout();
        if(layout) {
            layout->update();
        }
    }
}
/*!
    \internal
*/
void Widget::draw(CommandBuffer &buffer, uint32_t layer) {
    if(m_parent == nullptr && layer == CommandBuffer::UI && m_transform) {
        m_transform->setSize(buffer.viewport());
    }
}
/*!
    Lowers the widget to the bottom of the widget's stack.

    \sa raise()
*/
void Widget::lower() {
    RenderSystem *render = static_cast<RenderSystem *>(system());

    auto &widgets = render->widgets();
    widgets.remove(this);
    widgets.push_front(this);
}
/*!
    Raises this widget to the top of the widget's stack.

    \sa lower()
*/
void Widget::raise() {
    RenderSystem *render = static_cast<RenderSystem *>(system());

    auto &widgets = render->widgets();
    widgets.remove(this);
    widgets.push_back(this);
}
/*!
    \internal
*/
AABBox Widget::bound() const {
    AABBox result;

    if(m_transform) {
        Vector2 size(m_transform->size());
        result.extent = Vector3(size * 0.5f, 0.0f);
        result.center = result.extent;
    } else {
        result.extent = Vector3();
    }

    return result * transform()->worldTransform();
}

void Widget::boundChanged(const Vector2 &size) {
    A_UNUSED(size);
}
/*!
    Returns parent Widget.
*/
Widget *Widget::parentWidget() {
    return m_parent;
}
/*!
    Returns true if widget is visible on the screen.
*/
bool Widget::isVisible() const {
    return actor()->isEnabled();
}
/*!
    Returns RectTransform component attached to parent Actor.
*/
RectTransform *Widget::rectTransform() const {
    return m_transform;
}
/*!
    Returns the application widget that has the keyboard input focus, or nullptr if no widget in this application has the focus.
*/
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
/*!
    \internal
*/
void Widget::setParent(Object *parent, int32_t position, bool force) {
    NativeBehaviour::setParent(parent, position, force);

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
/*!
    \internal
*/
void Widget::composeComponent() {
    setRectTransform(Engine::objectCreate<RectTransform>("RectTransform", actor()));
}
/*!
    \internal
*/
void Widget::setSystem(ObjectSystem *system) {
    Object::setSystem(system);

    RenderSystem *render = static_cast<RenderSystem *>(system);
    render->addWidget(this);
}

void Widget::drawGizmosSelected() {
    AABBox box = bound();
    Gizmos::drawWireBox(box.center, box.extent * 2.0f, Vector4(0.5f, 1.0f, 0.5f, 1.0f));
}
