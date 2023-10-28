#ifndef NODEWIDGET_H
#define NODEWIDGET_H

#include <components/gui/frame.h>

class GraphNode;
class NodePort;

class Label;
class GraphView;

class NodeWidget : public Frame {
    A_REGISTER(NodeWidget, Frame, Editor/Graph)

    A_METHODS(
        A_SIGNAL(NodeWidget::pressed),
        A_SIGNAL(NodeWidget::portPressed),
        A_SIGNAL(NodeWidget::portReleased)
    )

public:
    NodeWidget();

    void setView(GraphView *view);
    void setGraphNode(GraphNode *node);

    void setSelected(bool flag);

    Frame *title() const;
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
    Vector3 m_originNodePos;

    GraphNode *m_node;

    Label *m_label;

    Frame *m_title;

    Image *m_preview;
    Image *m_previewBtn;

    GraphView *m_view;

    Layout *m_callLayout;

    bool m_hovered;

};

#endif // NODEWIDGET_H
