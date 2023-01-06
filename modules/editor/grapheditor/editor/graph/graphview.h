#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <editor/viewport/viewport.h>

#include <editor/graph/abstractnodegraph.h>

class QMenu;

class ObjectObserver;
class NodeWidget;
class LinksRender;
class Frame;

class NODEGRAPH_EXPORT GraphView : public Viewport {
    Q_OBJECT

public:
    explicit GraphView(QWidget *parent = nullptr);

    void setWorld(World *scene) override;

    AbstractNodeGraph *graph() const;
    void setGraph(AbstractNodeGraph *graph, bool state = false);

    void focusNode(NodeWidget *widget);

    void createLink(NodeWidget *node, int port);
    void buildLink(NodeWidget *node, int port);
    void deleteLink(NodeWidget *node, int port);

    void composeLinks();

    void reselect();

signals:
    void itemsSelected(const QList<QObject *> &);

private slots:
    void onComponentSelected();

    void onGraphUpdated();

    void onDraw() override;

private:
    bool isSelected(NodeWidget *widget) const;

    bool eventFilter(QObject *object, QEvent *event) override;

protected:
    Vector3 m_originMousePos;
    Vector3 m_originNodePos;
    Vector2 m_rubberOrigin;

    QList<QObject *> m_selectedItems;

    Scene *m_scene;

    QMenu *m_createMenu;

    AbstractNodeGraph *m_graph;

    ObjectObserver *m_objectObserver;

    LinksRender *m_linksRender;

    Frame *m_rubberBand;

    NodeWidget *m_focusedNode;

    bool m_drag;

};

class MoveNodes : public UndoGraph {
public:
    MoveNodes(const list<NodeWidget *> &selection, GraphView *view, const QString &name = QObject::tr("Move Node"), QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
private:
    GraphView *m_view;
    vector<int> m_indices;
    vector<Vector2> m_points;

};

#endif // GRAPHVIEW_H
