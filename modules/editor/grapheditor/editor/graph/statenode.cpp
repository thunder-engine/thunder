#include "statenode.h"

#include "graphwidgets/nodewidget.h"

#include <recttransform.h>
#include <layout.h>

Widget *StateNode::widget() {
    if(m_nodeWidget == nullptr) {
        Actor *nodeActor = Engine::composeActor<NodeWidget>(name());
        if(nodeActor) {
            NodeWidget *nodeWidget = nodeActor->getComponent<NodeWidget>();

            RectTransform *rect = nodeWidget->rectTransform();
            rect->setVerticalPolicy(RectTransform::Fixed);
            rect->setHorizontalPolicy(RectTransform::Fixed);

            Frame *title = nodeWidget->title();
            Vector4 corners(title->corners());
            corners.z = corners.w = corners.x;
            title->setCorners(corners);

            RectTransform *titleRect = title->rectTransform();
            titleRect->setMargin(Vector4(5.0f));
            titleRect->setAnchors(Vector2(0.0f), Vector2(1.0f));

            Layout *layout = rect->layout();
            layout->invalidate();
            layout->update();

            nodeWidget->setGraphNode(this);

            m_nodeWidget = nodeWidget;
        }
    }

    return m_nodeWidget;
}
