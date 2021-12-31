#ifndef SCHEMEVIEW_H
#define SCHEMEVIEW_H

#include <QQuickWidget>

#include <engine.h>

class AbstractSchemeModel;
class QMenu;

class NEXT_LIBRARY_EXPORT SchemeView : public QQuickWidget {
    Q_OBJECT

public:
    explicit SchemeView(QWidget *parent = nullptr);

    void setModel(AbstractSchemeModel *model, bool state = false);

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

    AbstractSchemeModel *m_model;

    QObject *m_selectedItem;

    int m_node;
    int m_port;
    bool m_out;

};

#endif // SCHEMEVIEW_H
