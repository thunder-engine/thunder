#ifndef COMPONENTBROWSER_H
#define COMPONENTBROWSER_H

#include <QTreeView>

class ComponentModel;
class ComponentFilter;

class Engine;

static const QString gMimeComponent("text/component");

namespace Ui {
    class ComponentBrowser;
}

class ComponentView : public QTreeView {
    Q_OBJECT

public:
    ComponentView(QWidget *parent) :
            QTreeView(parent) {
    }

signals:
    void dragStarted(Qt::DropActions supportedActions);

protected:
    void startDrag(Qt::DropActions supportedActions) {
        emit dragStarted(supportedActions);
    }
};

class ComponentBrowser : public QWidget {
    Q_OBJECT
public:
    ComponentBrowser(QWidget *parent = 0);

    void setGroups(const QStringList &groups = QStringList());

    void setModel(QAbstractItemModel *model);

signals:
    void componentSelected(const QString &uri);

private slots:
    void onDragStarted(Qt::DropActions supportedActions);

    void on_findComponent_textChanged(const QString &arg1);

    void on_componentsTree_clicked(const QModelIndex &index);

private:
    void changeEvent(QEvent *event) override;

    Ui::ComponentBrowser   *ui;

    ComponentFilter        *m_pProxyModel;

};

#endif // COMPONENTBROWSER_H
