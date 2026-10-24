#include "statenode.h"

#include "graphwidgets/nodewidget.h"

#include <layout.h>

void StateNode::updateName() {
    if(m_nodeWidget) {
        NodeWidget *nodeWidget = static_cast<NodeWidget *>(m_nodeWidget);
        nodeWidget->updateName();
    }
}

Widget *StateNode::widget() {
    if(m_nodeWidget == nullptr) {
        NodeWidget *nodeWidget = Widget::createWidget<NodeWidget>(name());
        if(nodeWidget) {

            if(nodeWidget) {
                nodeWidget->setVerticalPolicy(NodeWidget::Fixed);

                Frame *header = nodeWidget->header();
                Vector4 corners(header->corners());
                corners.z = corners.w = corners.x;
                header->setCorners(corners);
                header->setVerticalPolicy(NodeWidget::Expanding);
                header->setMargin(Vector4(5.0f));

                nodeWidget->setGraphNode(this);

                m_nodeWidget = nodeWidget;
            }
        }
    }

    return m_nodeWidget;
}
