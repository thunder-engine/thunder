#include "portwidget.h"

#include "../graphnode.h"

#include <components/actor.h>
#include <components/recttransform.h>

#include <viewport/handles.h>

PortWidget::PortWidget() :
    m_port(nullptr),
    m_editor(nullptr) {

}

void PortWidget::setNodePort(NodePort *port) {
    m_port = port;

    RectTransform *rect = rectTransform();
    rect->setPosition(Vector3(0.0f, (-m_port->m_pos - 1) * rect->size().y, 0.0f));

    // Create editor
    m_editor = Engine::composeActor("TextInput", qPrintable(m_port->m_name), actor());
    Widget *widget = static_cast<Widget *>(m_editor->component("Widget"));
    if(widget) {
        RectTransform *rect = widget->rectTransform();
        rect->setOffsets(Vector2(3.0f, 0.0f), Vector2(3.0f, 0.0f));
        rect->setAnchors(Vector2(), Vector2(1.0f));
    }
}

void PortWidget::composeComponent() {
    Widget::composeComponent();

    RectTransform *rect = rectTransform();
    if(rect) {
        rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
        rect->setPivot(Vector2(0.0f, 1.0f));
    }
}

bool PortWidget::drawHandles(ObjectList &selected) {
    return Widget::drawHandles(selected);
}
