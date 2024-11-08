#include "statenode.h"

#include "graphwidgets/nodewidget.h"

#include <recttransform.h>

Widget *StateNode::widget() {
    if(m_nodeWidget == nullptr) {
        NodeWidget *result = static_cast<NodeWidget *>(GraphNode::widget());

        Frame *title = result->title();
        Vector4 corners(title->corners());
        corners.z = corners.w = corners.x;
        title->setCorners(corners);

        RectTransform *titleRect = title->rectTransform();
        titleRect->setSize(Vector2(0.0f));
        titleRect->setMargin(Vector4(5.0f));
        titleRect->setAnchors(Vector2(0.0f), Vector2(1.0f));
    }

    return m_nodeWidget;
}
