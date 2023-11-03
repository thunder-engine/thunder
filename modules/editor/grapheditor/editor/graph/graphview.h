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
    void setGraph(AbstractNodeGraph *graph);

    Frame *rubberBand();

    void createLink(NodeWidget *node, int port);
    void buildLink(NodeWidget *node, int port);
    void deleteLink(NodeWidget *node, int port);

    bool isCreationLink() const;

    void composeLinks();

    void reselect();

    void showMenu();

signals:
    void itemsSelected(const QList<QObject *> &);

private slots:
    void onComponentSelected();

    void onGraphUpdated();

    void onDraw() override;

protected:
    Scene *m_scene;

    QMenu *m_createMenu;

    ObjectObserver *m_objectObserver;

    LinksRender *m_linksRender;

    Frame *m_rubberBand;

};

#endif // GRAPHVIEW_H
