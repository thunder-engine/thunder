#include "nodewidget.h"

#include "portwidget.h"

#include "../graphnode.h"
#include "../abstractnodegraph.h"
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

const float row = 20.0f;

namespace {
    const char *gPortWidget("PortWidget");
    const char *gImage("Image");
    const char *gFrame("Frame");
    const char *gToolButton("ToolButton");
};

NodeWidget::NodeWidget() :
        m_node(nullptr),
        m_label(nullptr),
        m_title(nullptr),
        m_callLayout(nullptr),
        m_hovered(false) {

}

void NodeWidget::setGraphNode(GraphNode *node) {
    m_node = node;

    updateName();

    if(m_title) {
        m_title->setColor(m_node->color());
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
    if(m_label) {
        std::string title = !m_node->objectName().isEmpty() ? qPrintable(m_node->objectName()) : m_node->typeName();
        m_label->setText(title);
    }
}

void NodeWidget::setSelected(bool flag) {
    if(flag) {
       setBorderColor(Vector4(1.0f));
    } else {
       setBorderColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}

Frame *NodeWidget::title() const {
    return m_title;
}

Label *NodeWidget::label() const {
    return m_label;
}

GraphNode *NodeWidget::node() const {
    return m_node;
}

void NodeWidget::update() {
    Widget::update();

    Vector4 pos = Input::mousePosition();

    if(m_title) {
        RectTransform *hoveredRect = rectTransform()->hoveredTransform(pos.x, pos.y);

        if(hoveredRect == m_title->rectTransform() && Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
            emitSignal(_SIGNAL(pressed()));
        } else if(m_node) {
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
                    emitSignal(_SIGNAL(portPressed(int)), -1);
                }
                if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
                    emitSignal(_SIGNAL(portReleased(int)), -1);
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

    Actor *title = Engine::composeActor(gFrame, "Title", actor());
    if(title) {
        m_title = static_cast<Frame *>(title->component(gFrame));
        if(m_title) {
            RectTransform *titleRect = m_title->rectTransform();
            layout->addTransform(titleRect);

            titleRect->setSize(Vector2(0, row));
            titleRect->setPivot(Vector2(0.0f, 1.0f));
            titleRect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));

            Vector4 corn(corners());
            corn.x = corn.y;
            corn.w = corn.z = 0.0f;
            m_title->setCorners(corn);
            m_title->setBorderColor(Vector4());

            m_label = Engine::objectCreate<Label>("", title);
            if(m_label) {
                m_label->setFontSize(14);
                m_label->setAlign(Alignment::Middle | Alignment::Center);
                m_label->setColor(Vector4(1.0f));
                m_label->setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
            }
        }
    }
}

void NodeWidget::composePort(NodePort &port) {
    Actor *portActor = Engine::composeActor(gPortWidget, port.m_name, actor());
    if(portActor) {

        PortWidget *portWidget = static_cast<PortWidget *>(portActor->component(gPortWidget));
        if(portWidget) {
            RectTransform *portRect = portWidget->rectTransform();
            portRect->setSize(Vector2(0, row));
            portWidget->setNodePort(&port);

            Layout *layout = rectTransform()->layout();
            if(layout) {
                if(port.m_call) {
                    if(m_callLayout) {
                        if(port.m_out) {
                            m_callLayout->insertTransform(-1, portRect);
                        } else {
                            m_callLayout->insertTransform(0, portRect);
                        }
                    } else {
                        m_callLayout = new Layout;
                        m_callLayout->setDirection(Layout::Horizontal);
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
