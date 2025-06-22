#ifndef NODEWIDGET_H
#define NODEWIDGET_H

#include <frame.h>

class GraphNode;
class NodePort;

class Image;
class Label;
class Layout;
class GraphView;

class NodeWidget : public Frame {
    A_OBJECT(NodeWidget, Frame, Editor/Graph)

    A_METHODS(
        A_SIGNAL(NodeWidget::pressed),
        A_SIGNAL(NodeWidget::portPressed),
        A_SIGNAL(NodeWidget::portReleased)
    )

public:
    NodeWidget();

    void setGraphNode(GraphNode *node);

    void updateName();

    bool isSelected() const;
    void setSelected(bool flag);

    Frame *title() const;

    Label *label() const;

    GraphNode *node() const;

public: //signals
    void pressed();
    void portPressed(int port);
    void portReleased(int port);

protected:
    void update() override;

    void composeComponent() override;

    void composePort(NodePort &port);

protected:
    GraphNode *m_node;

    Label *m_label;

    Frame *m_title;

    Layout *m_callLayout;

    bool m_hovered;

    bool m_selected;

};

#endif // NODEWIDGET_H
