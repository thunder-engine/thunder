#ifndef NODEWIDGET_H
#define NODEWIDGET_H

#include <components/gui/frame.h>

class GraphNode;
class NodePort;

class Label;

class NodeWidget : public Frame {
    A_REGISTER(NodeWidget, Frame, Editor/UI)

    A_METHODS(
        A_SIGNAL(NodeWidget::pressed),
        A_SIGNAL(NodeWidget::portPressed),
        A_SIGNAL(NodeWidget::portReleased)
    )

public:
    NodeWidget();

    void setGraphNode(GraphNode *node);

    Frame *title() const;
    GraphNode *node() const;

    void pressed();
    void portPressed(int port);
    void portReleased(int port);

private:
    void update() override;

    void composeComponent() override;

    void composePort(NodePort &port);

private:
    GraphNode *m_node;

    Label *m_label;

    Frame *m_title;

    bool m_hovered;

};

#endif // NODEWIDGET_H
