#include "nodewidget.h"

#include "portwidget.h"

#include "../graphnode.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <components/label.h>
#include <components/recttransform.h>

#include <resources/material.h>
#include <resources/font.h>

const float width = 200.0f;
const float height = 16.0f;
const float row = 20.0f;

NodeWidget::NodeWidget() :
    m_label(nullptr),
    m_title(nullptr) {

}

void NodeWidget::setGraphNode(GraphNode *node) {
    if(m_label) {
        QString title = !node->objectName().isEmpty() ? node->objectName() : node->m_type;
        m_label->setText(qPrintable(title));
    }

    float h = (node->m_ports.size() + 2) * row;

    RectTransform *rect = rectTransform();
    rect->setSize(Vector2(width, h));
    rect->setPosition(Vector3(node->m_pos.x(), node->m_pos.y() - h, 0.0f));

    for(auto port : qAsConst(node->m_ports)) {
        Actor *portActor = Engine::composeActor("PortWidget", qPrintable(port->m_name), actor());
        if(portActor) {
            PortWidget *widget = dynamic_cast<PortWidget *>(portActor->component("PortWidget"));
            if(widget) {
                widget->rectTransform()->setSize(Vector2(0, row));
                widget->setNodePort(port);
            }
        }
        return;
    }
}

void NodeWidget::composeComponent() {
    Frame::composeComponent();

    Actor *title = Engine::composeActor("Frame", "Title", actor());
    if(title) {
        m_title = static_cast<Frame *>(title->component("Frame"));
        if(m_title) {
            float border = 1.0f;
            RectTransform *rect = m_title->rectTransform();
            rect->setOffsets(Vector2(border, 0.0f), Vector2(border, 0.0f));
            rect->setAnchors(Vector2(0.0f, 1.0f), Vector2(1.0f, 1.0f));
            rect->setSize(Vector2(0, row - border));
            rect->setPivot(Vector2(0.0f, 1.0f));
            rect->setPosition(Vector3(0.0f, -border, 0.0));

            Vector4 corn(corners());
            corn.x = corn.y = 0.0f;
            corn.z = corn.w = corn.z - border;
            m_title->setCorners(corn);
            m_title->setColor(Vector4(0.235f, 0.113f, 0.149f, 1.0));
            m_title->setBorderWidth(0.0f);

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
