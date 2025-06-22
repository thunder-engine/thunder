#ifndef GRAPHCONTROLLER_H
#define GRAPHCONTROLLER_H

#include <editor/viewport/cameracontroller.h>

#include <editor/graph/abstractnodegraph.h>

class NodeWidget;
class GraphView;

class GraphController : public CameraController {
    Q_OBJECT

public:
    explicit GraphController(GraphView *view);

    AbstractNodeGraph *graph();
    void setGraph(AbstractNodeGraph *graph);

    Object::ObjectList selected() override;
    void selectNodes(const std::list<int32_t> &nodes);

    void composeLinks();

    void copySelected();
    const std::string &copyData() const;

    void onSelectNodes(const std::list<int32_t> &nodes, bool additive = false);

signals:
    void copied();

    void propertyChanged(const Object::ObjectList &objects, QString property, const Variant &value);

private:
    void update() override;

    void cameraMove(const Vector3 &delta) override;

    void cameraZoom(float delta) override;

    void resize(int32_t width, int32_t height) override;

    void rubberBandBehavior(const Vector2 &pos);
    void deleteNode();

    void cancelDrag();
    void beginDrag();

private:
    std::list<int32_t> m_selected;
    std::list<int32_t> m_softSelected;

    std::string m_copyData;

    Vector2 m_originMousePos;

    std::list<Vector2> m_selectedOrigins;
    std::list<Vector2> m_softOrigins;

    Widget *m_dragWidget;

    AbstractNodeGraph *m_graph;

    GraphView *m_view;

    int m_zoom;

};

#endif // GRAPHCONTROLLER_H
