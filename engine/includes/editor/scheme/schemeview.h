#ifndef SCHEMEVIEW_H
#define SCHEMEVIEW_H

#include <QQuickWidget>

#include <engine.h>

class AbstractNodeGraph;
class QMenu;

class ENGINE_EXPORT SchemeView : public QQuickWidget {
    Q_OBJECT

public:
    explicit SchemeView(QWidget *parent = nullptr);

    void setModel(AbstractNodeGraph *graph, bool state = false);

    void reselect();

public slots:
    void onNodesSelected(const QVariant &indices);

signals:
    void itemSelected(QObject *);

private slots:
    void onComponentSelected();

    void onShowContextMenu(int node, int port, bool out);

protected:
    QMenu *m_createMenu;

    AbstractNodeGraph *m_graph;

    QObject *m_selectedItem;

    int m_node;
    int m_port;
    bool m_out;

};

#endif // SCHEMEVIEW_H
