#include "nodegroup.h"

#include "graphwidgets/groupwidget.h"

#include <components/recttransform.h>
#include <components/label.h>

namespace  {
    const char *gGroupWidget("GroupWidget");
}

NodeGroup::NodeGroup() :
        m_color(Vector4(1.0f)),
        m_size(Vector2(400.0f, 200.0f)) {

}

std::string NodeGroup::text() const {
    return name();
}
void NodeGroup::setText(const std::string &text) {
    setName(text);
}

Vector4 NodeGroup::groupColor() const {
    return m_color;
}
void NodeGroup::setGroupColor(const Vector4 &color) {
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
    return m_color;
}

Widget *NodeGroup::widget() {
    if(m_nodeWidget == nullptr) {
        Actor *nodeActor = Engine::composeActor(gGroupWidget, name());
        if(nodeActor) {
            GroupWidget *nodeWidget = nodeActor->getComponent<GroupWidget>();

            if(nodeWidget) {
                nodeWidget->setGraphNode(this);

                m_nodeWidget = nodeWidget;

                RectTransform *rect = m_nodeWidget->rectTransform();
                rect->setPosition(Vector3(position().x, -position().y - rect->size().y, 0.0f));
            }
        }
    }

    return m_nodeWidget;
}
