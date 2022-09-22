#ifndef NODEWIDGET_H
#define NODEWIDGET_H

#include <components/frame.h>

class GraphNode;

class Label;

class NodeWidget : public Frame {
    A_REGISTER(NodeWidget, Frame, Editor/UI)

public:
    NodeWidget();

    void setGraphNode(GraphNode *node);

private:
    void composeComponent() override;

private:
    Label *m_label;

    Frame *m_title;

};

#endif // NODEWIDGET_H
