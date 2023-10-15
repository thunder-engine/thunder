#include "portwidget.h"

#include "../graphnode.h"
#include "../graphcontroller.h"
#include "../abstractnodegraph.h"

#include <components/actor.h>
#include <components/textrender.h>
#include <components/gui/recttransform.h>
#include <components/gui/label.h>

#include <input.h>

namespace {
    const char *gLabel("Label");
    const char *gWidget("Widget");
};

PortWidget::PortWidget() :
    m_port(nullptr),
    m_label(nullptr),
    m_editor(nullptr),
    m_knob(nullptr),
    m_hovered(false) {

}

void PortWidget::portUpdate() {
    GraphNode *node = m_port->m_node;
    if(node) {
        const auto link = node->graph()->findLink(node, m_port);
        m_label->setEnabled(link != nullptr || m_editor == nullptr);
        if(m_editor) {
            m_editor->setEnabled(link == nullptr);
        }
    } else {
        m_label->setEnabled(true);
        if(m_editor) {
            m_editor->setEnabled(false);
        }
    }
}

NodePort *PortWidget::port() const {
    return m_port;
}

void PortWidget::setNodePort(NodePort *port) {
    m_port = port;
    m_port->m_userData = this;

    RectTransform *rect = rectTransform();
    if(m_knob) {
        float knobSize = rect->size().y - 6.0f;
        rect = m_knob->rectTransform();
        rect->setSize(Vector2(port->m_call ? knobSize + 4 : knobSize, knobSize));
        rect->setAnchors(Vector2(m_port->m_out ? 1.0f : 0.0f, 0.5f), Vector2(m_port->m_out ? 1.0f : 0.0f, 0.5f));

        if(port->m_call) {
            m_knob->setCorners(Vector4(knobSize * 0.1f, knobSize * 0.9f, knobSize * 0.9f, knobSize * 0.1f));
        } else {
            m_knob->setCorners(Vector4(knobSize * 0.5f));
        }
        m_knob->setColor(m_port->m_color);
        m_knob->setBorderWidth(0.0f);
    }

    // Create editor (only for inputs)
    if(!m_port->m_out) {
        if(m_editor) {
            Widget *widget = static_cast<Widget *>(m_editor->component(gWidget));

            rect = widget->rectTransform();
            rect->setOffsets(Vector2(10.0f, 0.0f), Vector2(10.0f, 0.0f));
            rect->setAnchors(Vector2(), Vector2(1.0f));
        }
    }

    m_label = Engine::composeActor(gLabel, m_port->m_name.c_str(), actor());
    Label *label = static_cast<Label *>(m_label->component(gLabel));
    label->setText(m_port->m_name.c_str());
    label->setAlign(Alignment::Middle | (m_port->m_out ? Alignment::Right : Alignment::Left));
    label->setColor(Vector4(1.0f));

    rect = label->rectTransform();
    rect->setOffsets(Vector2(10.0f, 0.0f), Vector2(10.0f, 0.0f));
    rect->setAnchors(Vector2(), Vector2(1.0f));

    portUpdate();
}

Frame *PortWidget::knob() const {
    return m_knob;
}

void PortWidget::update() {
    if(m_knob) {
        Vector3 pos = GraphController::worldPosition();

        bool hover = m_knob->rectTransform()->isHovered(pos.x, pos.y);
        if(m_hovered != hover) {
            m_hovered = hover;

            Vector4 color(m_port->m_color);
            if(m_hovered) {
                color.x = CLAMP(color.x + 0.25f, 0.0f, 1.0f);
                color.y = CLAMP(color.y + 0.25f, 0.0f, 1.0f);
                color.z = CLAMP(color.z + 0.25f, 0.0f, 1.0f);
                color.w = CLAMP(color.w + 0.25f, 0.0f, 1.0f);
            }
            m_knob->setColor(color);
        }

        if(m_hovered) {
            if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
                emitSignal(_SIGNAL(pressed(int)), m_port->m_node->portPosition(m_port));
            }
            if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
                emitSignal(_SIGNAL(released(int)), m_port->m_node->portPosition(m_port));
            }
        }
    }
}

void PortWidget::composeComponent() {
    Widget::composeComponent();

    RectTransform *rect = rectTransform();
    if(rect) {
        rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
        rect->setPivot(Vector2(0.0f, 1.0f));
    }

    Actor *knob = Engine::composeActor("Frame", "Knob", actor());
    m_knob = static_cast<Frame *>(knob->component("Frame"));
}
