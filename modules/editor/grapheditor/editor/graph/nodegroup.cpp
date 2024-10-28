#include "nodegroup.h"

#include "graphwidgets/groupwidget.h"

#include <components/recttransform.h>
#include <components/label.h>

namespace  {
    const char *gGroupWidget("GroupWidget");
}

NodeGroup::NodeGroup() :
        m_color(QColor(255, 255, 255, 255)),
        m_size(Vector2(400.0f, 200.0f)) {

}

QString NodeGroup::text() const {
    return objectName();
}
void NodeGroup::setText(const QString &text) {
    setObjectName(text);
}

QColor NodeGroup::groupColor() const {
    return m_color;
}
void NodeGroup::setGroupColor(const QColor &color) {
    m_color = color;
}

float NodeGroup::width() const {
    return m_size.x;
}
void NodeGroup::setWidth(const float width) {
    m_size.x = width;
}

float NodeGroup::height() const {
    return m_size.y;
}
void NodeGroup::setHeight(const float height) {
    m_size.y = height;
}

Vector2 NodeGroup::defaultSize() const {
    return m_size;
}

Vector4 NodeGroup::color() const {
    return Vector4(m_color.redF(), m_color.greenF(),
                   m_color.blueF(), m_color.alphaF());
}

Widget *NodeGroup::widget() {
    if(m_nodeWidget == nullptr) {
        Actor *nodeActor = Engine::composeActor(gGroupWidget, qPrintable(objectName()));
        if(nodeActor) {
            GroupWidget *nodeWidget = static_cast<GroupWidget *>(nodeActor->component(gGroupWidget));

            if(nodeWidget) {
                nodeWidget->setGraphNode(this);
                nodeWidget->setBorderColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));

                m_nodeWidget = nodeWidget;

                RectTransform *rect = m_nodeWidget->rectTransform();
                rect->setPosition(Vector3(position().x, -position().y - rect->size().y, 0.0f));
            }
        }
    }

    return m_nodeWidget;
}
