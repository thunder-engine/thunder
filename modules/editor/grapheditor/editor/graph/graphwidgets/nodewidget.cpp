#include "nodewidget.h"

#include "portwidget.h"

#include "../graphnode.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <components/gui/label.h>
#include <components/gui/recttransform.h>
#include <components/gui/layout.h>

#include <resources/material.h>
#include <resources/font.h>

#include <input.h>

const float row = 20.0f;

namespace {
    const char *gPortWidget("PortWidget");
};

NodeWidget::NodeWidget() :
        m_node(nullptr),
        m_label(nullptr),
        m_title(nullptr),
        m_hovered(false) {

}

void NodeWidget::setGraphNode(GraphNode *node) {
    m_node = node;
    if(m_label) {
        string title = !m_node->objectName().isEmpty() ? qPrintable(node->objectName()) : node->type();
        m_label->setText(title);
    }

    if(m_title) {
        m_title->setColor(m_node->color());
        if(m_node->isState()) {
            rectTransform()->layout()->setMargins(0.0f, 10.0f, 10.0f, 0.0f);

            RectTransform *rect = m_title->rectTransform();
            rect->setOffsets(Vector2(10.0f), Vector2(10.0f));

            Vector4 corn = m_title->corners();
            corn.x = corn.y = corn.z = corn.w = 5.0f;
            m_title->setCorners(corn);
        }
    }

    // Out ports
    for(NodePort &port : m_node->ports()) {
        if(port.m_out) {
            composePort(port);
        }
    }

    // Add properties
    if(!m_node->isState()) {

    }

    // In ports
    for(NodePort &port : m_node->ports()) {
        if(!port.m_out) {
            composePort(port);
        }
    }

    RectTransform *rect = rectTransform();

    Vector2 size = m_node->defaultSize();
    if(!m_node->ports().empty()) {
        size.y = (m_node->ports().size() + 2.0f) * row;
        Layout *layout = rect->layout();
        if(layout) {
            size.y = layout->sizeHint().y;
            layout->update();
        }
    }

    rect->setSize(size);
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

GraphNode *NodeWidget::node() const {
    return m_node;
}

void NodeWidget::update() {
    Widget::update();

    Vector4 pos = Input::mousePosition();
    if(m_title) {
        bool hover = m_title->rectTransform()->isHovered(pos.x, pos.y);
        if(hover && Input::isMouseButtonDown(0)) {
            emitSignal(_SIGNAL(pressed()));
        } else {
            if(m_node && m_node->isState()) { // For state machine
                bool hover = rectTransform()->isHovered(pos.x, pos.y);
                if(m_hovered != hover) {
                    m_hovered = hover;

                    Vector4 color(m_color);
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
}

void NodeWidget::composeComponent() {
    Frame::composeComponent();

    setColor(Vector4(0.376f, 0.376f, 0.376f, 1.0f));

    Layout *layout = new Layout;
    layout->setSpacing(2.0f);
    layout->setMargins(0.0f, 0.0f, 0.0f, corners().x);
    rectTransform()->setLayout(layout);

    Actor *title = Engine::composeActor("Frame", "Title", actor());
    if(title) {
        m_title = static_cast<Frame *>(title->component("Frame"));
        if(m_title) {
            RectTransform *rect = m_title->rectTransform();
            rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
            rect->setSize(Vector2(0, row));
            rect->setPivot(Vector2(0.0f, 1.0f));

            layout->addTransform(rect);

            Vector4 corn(corners());
            corn.x = corn.y = 0.0f;
            corn.w = corn.z;
            m_title->setCorners(corn);
            m_title->setBorderColor(Vector4());

            m_label = static_cast<Label *>(Engine::objectCreate("Label", "", title));
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
    Actor *portActor = Engine::composeActor(gPortWidget, port.m_name.c_str(), actor());
    if(portActor) {
        PortWidget *widget = static_cast<PortWidget *>(portActor->component(gPortWidget));
        if(widget) {
            widget->rectTransform()->setSize(Vector2(0, row));
            widget->setNodePort(&port);
            rectTransform()->layout()->addTransform(widget->rectTransform());
            connect(widget, _SIGNAL(pressed(int)), this, _SIGNAL(portPressed(int)));
            connect(widget, _SIGNAL(released(int)), this, _SIGNAL(portReleased(int)));
        }
    }
}
