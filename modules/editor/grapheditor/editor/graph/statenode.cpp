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
                rect->setHorizontalPolicy(RectTransform::Fixed);

                Frame *header = nodeWidget->header();
                Vector4 corners(header->corners());
                corners.z = corners.w = corners.x;
                header->setCorners(corners);


                RectTransform *titleRect = header->rectTransform();
                titleRect->setMargin(Vector4(5.0f));
                titleRect->setAnchors(Vector2(0.0f), Vector2(1.0f));

                Layout *layout = rect->layout();
                layout->invalidate();
                layout->update();

                nodeWidget->setGraphNode(this);

                m_nodeWidget = nodeWidget;
            }
        }
    }

    return m_nodeWidget;
}
