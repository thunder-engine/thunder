#ifndef HIERARCHYBROWSER_H
#define HIERARCHYBROWSER_H

#include <QWidget>
#include <QTreeView>
#include <QMenu>

#include <object.h>

namespace Ui {
    class HierarchyBrowser;
}

class ObjectCtrl;
class ObjectHierarchyModel;

class TreeView : public QTreeView {
    Q_OBJECT

public:
    TreeView(QWidget *parent) :
            QTreeView(parent) {
    }

signals:
    void dragStarted(Qt::DropActions supportedActions);
    void dragEnter(QDragEnterEvent *e);
    void dragLeave(QDragLeaveEvent *e);
    void dragMove(QDragMoveEvent *e);
    void drop(QDropEvent *e);

protected:
    void startDrag(Qt::DropActions supportedActions) { emit dragStarted(supportedActions); }
    void dragEnterEvent(QDragEnterEvent *e) { emit dragEnter(e); }
    void dragLeaveEvent(QDragLeaveEvent *e) { emit dragLeave(e); }
    void dragMoveEvent(QDragMoveEvent *e) { emit dragMove(e); }
    void dropEvent(QDropEvent *e) { emit drop(e); }
};

class ObjectsFilter;

class HierarchyBrowser : public QWidget {
    Q_OBJECT

public:
    HierarchyBrowser(QWidget *parent = 0);
    ~HierarchyBrowser();

    void setRootObject(Object *object);
    void setSimplified(bool enable);
    void setComponentsFilter(const QStringList &list);
    void setController(ObjectCtrl *ctrl);

    Object *findObject(uint32_t id);

signals:
    void selected(Object::ObjectList objects);
    void removed(Object::ObjectList objects);
    void focused(Object *object);
    void parented(Object::ObjectList objects, Object *parent);
    void updated();

public slots:
    void onObjectSelected(Object::ObjectList objects);
    void onHierarchyUpdated();

    void onDragEnter(QDragEnterEvent *e);
    void onDragLeave(QDragLeaveEvent *);
    void onDragMove(QDragMoveEvent *e);
    void onDrop(QDropEvent *e);

private slots:
    void onDragStarted(Qt::DropActions supportedActions);

    void on_treeView_clicked(const QModelIndex &index);
    void on_treeView_doubleClicked(const QModelIndex &index);
    void on_treeView_customContextMenuRequested(const QPoint &pos);
    void on_lineEdit_textChanged(const QString &arg1);

    void onCreateActor();

    void onItemDuplicate();
    void onItemRename();
    void onItemDelete();

    void onItemUnpack();
    void onItemUnpackAll();

private:
    QAction *createAction(const QString &name, const char *member, const QKeySequence &shortcut = 0);

    void changeEvent(QEvent *event) override;

private:
    QMenu m_ContentMenu;

    Ui::HierarchyBrowser *ui;
    QRubberBand *m_pRect;
    ObjectsFilter *m_pFilter;
    ObjectCtrl *m_pController;

    QList<QAction *> m_Prefab;
};

#endif // HIERARCHYBROWSER_H
