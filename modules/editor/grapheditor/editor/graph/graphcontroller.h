#ifndef GRAPHCONTROLLER_H
#define GRAPHCONTROLLER_H

#include <editor/viewport/cameracontroller.h>

#include <editor/graph/abstractnodegraph.h>

class NodeWidget;
class GraphView;

class GraphController : public CameraController {
public:
    explicit GraphController(GraphView *view);

    NodeWidget *focusNode();
    void setFocusNode(NodeWidget *widget);

    AbstractNodeGraph *graph();
    void setGraph(AbstractNodeGraph *graph);

    const std::list<QObject *> &selectedItems() const;
    void setSelected(const std::list<QObject *> &selected);

    void composeLinks();

private:
    void update() override;

    void cameraMove(const Vector3 &delta) override;

    void cameraZoom(float delta) override;

    void resize(int32_t width, int32_t height) override;

    bool isSelected(NodeWidget *widget) const;

private:
    std::list<QObject *> m_selectedItems;
    std::list<QObject *> m_softSelectedItems;

    Vector3 m_originMousePos;
    Vector3 m_originNodePos;
    Vector2 m_rubberOrigin;

    NodeWidget *m_focusedWidget;

    AbstractNodeGraph *m_graph;

    GraphView *m_view;

    int m_zoom;

    bool m_drag;

};

class MoveNodes : public UndoGraph {
public:
    MoveNodes(const std::list<NodeWidget *> &selection, GraphController *ctrl, const QString &name = QObject::tr("Move Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:

    GraphController *m_controller;

    std::vector<int> m_indices;
    std::vector<Vector2> m_points;

};

#endif // GRAPHCONTROLLER_H
