#ifndef GRAPHCONTROLLER_H
#define GRAPHCONTROLLER_H

#include <editor/viewport/cameracontroller.h>

#include <editor/graph/abstractnodegraph.h>
#include <editor/undostack.h>

class NodeWidget;
class PortWidget;

class GraphView;

class GraphController : public CameraController {
    Q_OBJECT

public:
    explicit GraphController(GraphView *view);

    AbstractNodeGraph *graph();
    void setGraph(AbstractNodeGraph *graph);

    Object::ObjectList selected() override;
    void selectNodes(const std::list<int32_t> &nodes);

    std::list<int32_t> selectedLinks() const;
    void selectLinks(const std::list<int32_t> &links);

    void composeLinks();

    void copySelected();
    const std::string &copyData() const;

    void onSelectNodes(const std::list<int32_t> &nodes, bool additive = false);
    void onSelectLinks(const std::list<int32_t> &links, bool additive = false);

signals:
    void copied();

private:
    void update() override;

    NodeWidget *hoveredNode(float mouseX, float mouseY);
    PortWidget *hoveredPort(float mouseX, float mouseY);
    int32_t hoveredLink(float mouseX, float mouseY);

    void cameraMove(const Vector3 &delta) override;

    void cameraZoom(float delta) override;

    void resize(int32_t width, int32_t height) override;

    void rubberBandBehavior(const Vector2 &pos);
    void deleteNodes();
    void deleteLinks();

    void dragSelection(const Vector2 &position);
    void cancelDrag();
    void beginDrag(NodeWidget *hovered);
    void endDrag();

private:
    std::list<int32_t> m_selectedNodes;
    std::list<int32_t> m_softSelected;
    std::list<int32_t> m_selectedLinks;

    std::string m_copyData;

    Vector2 m_originMousePos;

    std::list<Vector2> m_selectedOrigins;
    std::list<Vector2> m_softOrigins;

    Widget *m_dragWidget;

    AbstractNodeGraph *m_graph;

    GraphView *m_view;

    int32_t m_zoom;

};

#endif // GRAPHCONTROLLER_H
