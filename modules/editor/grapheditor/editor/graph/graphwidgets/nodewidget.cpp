#include "nodewidget.h"

#include "portwidget.h"

#include "../graphnode.h"
#include "../graphview.h"
#include "../graphcontroller.h"

#include <components/actor.h>
#include <components/textrender.h>
#include <components/spriterender.h>

#include <components/recttransform.h>
#include <components/layout.h>
#include <components/label.h>
#include <components/image.h>
#include <components/toolbutton.h>

#include <resources/material.h>
#include <resources/font.h>

#include <input.h>

#include "../statenode.h"

const float row = 20.0f;

NodeWidget::NodeWidget() :
        m_node(nullptr),
        m_title(nullptr),
        m_header(nullptr),
        m_callLayout(nullptr),
        m_hovered(false) {

}

void NodeWidget::setGraphNode(GraphNode *node) {
    m_node = node;

    updateName();

    if(m_header) {
        m_header->setColor(m_node->color());
    }

    rectTransform()->setSize(m_node->defaultSize());

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

void NodeWidget::update() {
    Widget::update();

    Vector4 pos = Input::mousePosition();

    if(m_header) {
        RectTransform *hoveredRect = rectTransform()->hoveredTransform(pos.x, pos.y);

        if(hoveredRect == m_header->rectTransform() && Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
            pressed();
        } else if(dynamic_cast<StateNode *>(m_node)) {
            bool hover = (hoveredRect == rectTransform());
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
                setColor(color);
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

    setColor(Vector4(0.376f, 0.376f, 0.376f, 1.0f));
    setCorners(8);

    Layout *layout = new Layout;
    layout->setSpacing(2.0f);

    RectTransform *rect = rectTransform();
    rect->setPivot(Vector2(0.0f, 1.0f));
    rect->setLayout(layout);
    rect->setVerticalPolicy(RectTransform::Preferred);
    rect->setHorizontalPolicy(RectTransform::Fixed);
    rect->setBorder(2.0f);

    Actor *header = Engine::composeActor<Frame>("Header", actor());
    if(header) {
        m_header = header->getComponent<Frame>();
        if(m_header) {
            RectTransform *headerRect = m_header->rectTransform();
            layout->addTransform(headerRect);

            headerRect->setSize(Vector2(0, row));
            headerRect->setPivot(Vector2(0.0f, 1.0f));
            headerRect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));

            Vector4 corn(corners() - 1);
            corn.x = corn.y;
            corn.w = corn.z = 0.0f;
            m_header->setCorners(corn);
            m_header->setBorderColor(Vector4());

            Actor *title = Engine::composeActor<Label>("Title", header);
            if(title) {
                m_title = title->getComponent<Label>();
            }
        }
    }

    setSelected(false);
}

void NodeWidget::composePort(NodePort &port) {
    Actor *portActor = Engine::composeActor<PortWidget>(port.m_name, actor());
    if(portActor) {
        PortWidget *portWidget = portActor->getComponent<PortWidget>();
        if(portWidget) {
            RectTransform *portRect = portWidget->rectTransform();
            portRect->setSize(Vector2(0, row));
            portWidget->setNodePort(&port);

            Layout *layout = rectTransform()->layout();
            if(layout) {
                if(port.m_call) {
                    if(port.m_out) {
                        portRect->setMinAnchors(Vector2(0.5f, 1.0f));
                    } else {
                        portRect->setMaxAnchors(Vector2(0.5f, 1.0f));
                    }
                    portRect->setPivot(Vector2(0.0f, 0.5f));

                    if(m_callLayout) {
                        if(port.m_out) {
                            m_callLayout->insertTransform(-1, portRect);
                        } else {
                            m_callLayout->insertTransform(0, portRect);
                        }
                    } else {
                        m_callLayout = new Layout;
                        m_callLayout->setOrientation(Widget::Horizontal);
                        m_callLayout->addTransform(portRect);
                        layout->addLayout(m_callLayout);
                    }
                } else {
                    layout->addTransform(portRect);
                    portRect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
                }
            }
            connect(portWidget, _SIGNAL(pressed(int)), this, _SIGNAL(portPressed(int)));
            connect(portWidget, _SIGNAL(released(int)), this, _SIGNAL(portReleased(int)));
        }
    }
}
