#include "statenode.h"

#include "graphwidgets/nodewidget.h"

#include <recttransform.h>
#include <layout.h>

void StateNode::updateName() {
    if(m_nodeWidget) {
        NodeWidget *nodeWidget = static_cast<NodeWidget *>(m_nodeWidget);
        nodeWidget->updateName();
    }
}

Widget *StateNode::widget() {
    if(m_nodeWidget == nullptr) {
        Actor *nodeActor = Engine::composeActor<NodeWidget>(name());
        if(nodeActor) {
            NodeWidget *nodeWidget = nodeActor->getComponent<NodeWidget>();

            if(nodeWidget) {
                RectTransform *rect = nodeWidget->rectTransform();
                rect->setVerticalPolicy(RectTransform::Fixed);

                Frame *header = nodeWidget->header();
                Vector4 corners(header->corners());
                corners.z = corners.w = corners.x;
                header->setCorners(corners);

                RectTransform *headerRect = header->rectTransform();
                headerRect->setVerticalPolicy(RectTransform::Expanding);
                headerRect->setMargin(Vector4(5.0f));

                nodeWidget->setGraphNode(this);

                m_nodeWidget = nodeWidget;
            }
        }
    }

    return m_nodeWidget;
}
