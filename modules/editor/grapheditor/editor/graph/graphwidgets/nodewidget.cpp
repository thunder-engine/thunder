#include "nodewidget.h"

#include "portwidget.h"

#include "../graphnode.h"
#include "../graphview.h"
#include "../graphcontroller.h"

#include <layout.h>
#include <label.h>
#include <image.h>
#include <toolbutton.h>

#include <resources/material.h>
#include <resources/font.h>

#include <input.h>

#include "../statenode.h"

const float row = 20.0f;

NodeWidget::NodeWidget() :
        m_node(nullptr),
        m_title(nullptr),
        m_header(nullptr),
        m_callRect(nullptr),
        m_hovered(false) {

}

void NodeWidget::setGraphNode(GraphNode *node) {
    m_node = node;

    updateName();

    if(m_header) {
        m_header->setBackgroundColor(m_node->color());
    }

    this->setSize(m_node->defaultSize());

    // Call ports
    for(NodePort &port : m_node->ports()) {
        if(port.m_call) {
            composePort(port);
        }
    }

    // Out ports
    for(NodePort &port : m_node->ports()) {
        if(port.m_out && !port.m_call) {
            composePort(port);
        }
    }

    // In ports
    for(NodePort &port : m_node->ports()) {
        if(!port.m_out && !port.m_call) {
            composePort(port);
        }
    }
}

void NodeWidget::updateName() {
    if(m_title) {
        m_title->setText(!m_node->name().isEmpty() ? m_node->name() : m_node->typeName());
    }
}

bool NodeWidget::isSelected() const {
    return m_selected;
}

void NodeWidget::setSelected(bool flag) {
    m_selected = flag;
    if(m_selected) {
       setBorderColor(Vector4(1.0f, 0.5f, 0.0f, 1.0f));
    } else {
       setBorderColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    }
}

Frame *NodeWidget::header() const {
    return m_header;
}

Label *NodeWidget::title() const {
    return m_title;
}

GraphNode *NodeWidget::node() const {
    return m_node;
}

void NodeWidget::pressed() {
    emitSignal(_SIGNAL(pressed()));
}

void NodeWidget::portPressed(int port) {
    emitSignal(_SIGNAL(portPressed(int)), port);
}

void NodeWidget::portReleased(int port) {
    emitSignal(_SIGNAL(portReleased(int)), port);
}

void NodeWidget::update(const Vector2 &pos) {
    Widget::update(pos);

    if(m_header) {
        if(m_header->isHovered(pos) && Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
            pressed();
        } else if(dynamic_cast<StateNode *>(m_node)) {
            bool hover = isHovered(pos);
            if(m_hovered != hover) {
                m_hovered = hover;

                Vector4 color(m_backgroundColor);
                if(m_hovered) {
                    color.x = CLAMP(color.x + 0.25f, 0.0f, 1.0f);
                    color.y = CLAMP(color.y + 0.25f, 0.0f, 1.0f);
                    color.z = CLAMP(color.z + 0.25f, 0.0f, 1.0f);
                } else {
                    color.x = CLAMP(color.x - 0.25f, 0.0f, 1.0f);
                    color.y = CLAMP(color.y - 0.25f, 0.0f, 1.0f);
                    color.z = CLAMP(color.z - 0.25f, 0.0f, 1.0f);
                }
                setBackgroundColor(color);
            }

            if(m_hovered) {
                if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
                    portPressed(-1);
                }
                if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
                    portReleased(-1);
                }
            }
        }
    }
}

void NodeWidget::composeComponent() {
    Frame::composeComponent();

    setBackgroundColor(Vector4(0.376f, 0.376f, 0.376f, 1.0f));
    setCorners(8);

    Layout *layout = new Layout;
    layout->setSpacing(2.0f);

    setPivot(Vector2(0.0f, 1.0f));
    setLayout(layout);
    setVerticalPolicy(NodeWidget::Preferred);
    setHorizontalPolicy(NodeWidget::Fixed);
    setBorder(2.0f);

    Frame *header = Widget::createWidget<Frame>("Header", this);
    if(header) {
        m_header = header;
        if(m_header) {
            layout->addTransform(m_header);

            m_header->setSize(Vector2(0, row));
            m_header->setPivot(Vector2(0.0f, 1.0f));
            m_header->setHorizontalPolicy(Frame::Expanding);
            m_header->setVerticalPolicy(Frame::Fixed);

            Vector4 corn(corners() - 1);
            corn.x = corn.y;
            corn.w = corn.z = 0.0f;
            m_header->setCorners(corn);
            m_header->setBorderColor(Vector4());

            Label *title = Widget::createWidget<Label>("Title", header);
            if(title) {
                m_title = title;
            }
        }
    }

    setSelected(false);
}

void NodeWidget::composePort(NodePort &port) {
    PortWidget *portWidget = Widget::createWidget<PortWidget>(port.m_name, this);
    if(portWidget) {
        if(portWidget) {
            portWidget->setSize(Vector2(0, row));
            portWidget->setHorizontalPolicy(PortWidget::Expanding);
            portWidget->setNodePort(&port);

            if(port.m_call) {
                if(m_callRect) {
                    Layout *layout = m_callRect->layout();
                    if(port.m_out) {
                        layout->insertTransform(-1, portWidget);
                    } else {
                        layout->insertTransform(0, portWidget);
                    }
                } else {
                    Widget *callWidget = Widget::createWidget<Widget>("CallPannel", this);
                    if(callWidget) {
                        m_callRect = callWidget;
                        if(m_callRect) {
                            m_callRect->setSize(Vector2(0, row));
                            m_callRect->setHorizontalPolicy(Widget::Expanding);
                            m_callRect->setVerticalPolicy(Widget::Fixed);

                            Layout *layout = new Layout;
                            layout->setOrientation(Widget::Horizontal);
                            m_callRect->setLayout(layout);

                            layout->addTransform(portWidget);
                        }
                    }
                }
            }

            connect(portWidget, _SIGNAL(pressed(int)), this, _SIGNAL(portPressed(int)));
            connect(portWidget, _SIGNAL(released(int)), this, _SIGNAL(portReleased(int)));
        }
    }
}
