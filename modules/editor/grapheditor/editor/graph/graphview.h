#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <editor/viewport/viewport.h>

#include <editor/graph/abstractnodegraph.h>

class QMenu;

class NODEGRAPH_EXPORT GraphView : public Viewport {
    Q_OBJECT

public:
    explicit GraphView(QWidget *parent = nullptr);

    void setGraph(AbstractNodeGraph *graph, bool state = false);

    void reselect();

public slots:
    void onNodesSelected(const QVariant &indices);

signals:
    void itemSelected(QObject *);

private slots:
    void onComponentSelected();

    void onGraphUpdated();

    void onDraw() override;

    void onShowContextMenu(int node, int port, bool out);

private:
    bool eventFilter(QObject *object, QEvent *event) override;

protected:
    QMenu *m_createMenu;

    AbstractNodeGraph *m_graph;

    QObject *m_selectedItem;

    int m_node;
    int m_port;
    bool m_out;

};

#endif // GRAPHVIEW_H
