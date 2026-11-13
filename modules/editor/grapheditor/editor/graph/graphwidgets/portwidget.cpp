#include "portwidget.h"

#include "../graphnode.h"
#include "../abstractnodegraph.h"

#include <label.h>

#include <input.h>

PortWidget::PortWidget() :
        m_port(nullptr),
        m_label(nullptr),
        m_editor(nullptr),
        m_knob(nullptr),
        m_hovered(false) {

}

PortWidget::~PortWidget() {
    if(m_port) {
        m_port->m_widget = nullptr;
    }
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
    m_port->m_widget = this;

    if(m_knob) {
        float knobSize = size().y - 6.0f;
        m_knob->setBorder(0.0f);
        m_knob->setSize(Vector2(port->m_call ? knobSize + 4 : knobSize, knobSize));
        m_knob->setAnchors(Vector2(m_port->m_out ? 1.0f : 0.0f, 0.5f), Vector2(m_port->m_out ? 1.0f : 0.0f, 0.5f));

        if(port->m_call) {
            m_knob->setCorners(Vector4(knobSize * 0.1f, knobSize * 0.9f, knobSize * 0.9f, knobSize * 0.1f));
        } else {
            m_knob->setCorners(Vector4(knobSize * 0.5f));
        }
        m_knob->setBackgroundColor(m_port->m_color);
    }

    // Create editor (only for inputs)
    if(!m_port->m_out) {
        if(m_editor) {
            m_editor->setMargin(Vector4(0.0f, 10.0f, 0.0f, 10.0f));
            m_editor->setAnchors(Vector2(), Vector2(1.0f));
        }
    }

    m_label = Widget::createWidget<Label>(m_port->m_name, this);
    m_label->setSize(Vector2());
    m_label->setMargin(Vector4(0.0f, 10.0f, 0.0f, 10.0f));
    m_label->setAnchors(Vector2(0.0f), Vector2(1.0f));
    m_label->setText(m_port->m_name);
    m_label->setAlign(Alignment::Middle | (m_port->m_out ? Alignment::Right : Alignment::Left));
    m_label->setColor(Vector4(1.0f));

    portUpdate();
}

Frame *PortWidget::knob() const {
    return m_knob;
}

void PortWidget::pressed(int position) {
    emitSignal(_SIGNAL(pressed(int)), position);
}

void PortWidget::released(int position) {
    emitSignal(_SIGNAL(released(int)), position);
}

void PortWidget::update(const Vector2 &pos) {
    if(m_knob) {
        bool hover = m_knob->isHovered(pos);
        if(m_hovered != hover) {
            m_hovered = hover;

            Vector4 color(m_port->m_color);
            if(m_hovered) {
                color.x = CLAMP(color.x + 0.25f, 0.0f, 1.0f);
                color.y = CLAMP(color.y + 0.25f, 0.0f, 1.0f);
                color.z = CLAMP(color.z + 0.25f, 0.0f, 1.0f);
                color.w = CLAMP(color.w + 0.25f, 0.0f, 1.0f);
            }
            m_knob->setBackgroundColor(color);
        }

        if(m_hovered) {
            if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
                pressed(m_port->m_node->portPosition(m_port));
            }
            if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
                released(m_port->m_node->portPosition(m_port));
            }
        }
    }
}

void PortWidget::composeComponent() {
    Widget::composeComponent();

    setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
    setPivot(Vector2(0.0f, 1.0f));

    m_knob = Widget::createWidget<Frame>("Knob", this);
}
