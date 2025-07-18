#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <editor/viewport/viewport.h>

#include <editor/graph/abstractnodegraph.h>

class QMenu;

class GraphViewProxy;
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

    Actor &view() const;

    Frame *rubberBand();

    void createLink(NodeWidget *node, int port);
    void buildLink(NodeWidget *node, int port);
    void deleteLink(NodeWidget *node, int port);

    bool isCreationLink() const;

    void composeLinks();

    void reselect();

    void showMenu();

    void onCutAction();
    void onCopyAction();
    void onPasteAction();

    bool isCopyActionAvailable() const;
    bool isPasteActionAvailable() const;

signals:
    void objectsSelected(const Object::ObjectList &);

    void copied();

public slots:
    void onObjectsChanged(const Object::ObjectList &objects, QString property, const Variant &value);

    void onGraphUpdated();

    void onGraphLoaded();

private slots:
    void onComponentSelected();

    void onDraw() override;

private:
    void resizeEvent(QResizeEvent *event) override;

protected:
    Scene *m_scene;

    Actor *m_view;

    QMenu *m_createMenu;

    GraphViewProxy *m_proxy;

    LinksRender *m_linksRender;

    Frame *m_rubberBand;

    bool m_updateLinks;

};

#endif // GRAPHVIEW_H
