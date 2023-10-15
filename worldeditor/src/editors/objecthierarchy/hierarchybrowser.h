#ifndef HIERARCHYBROWSER_H
#define HIERARCHYBROWSER_H

#include <QWidget>
#include <QTreeView>
#include <QMenu>

#include <object.h>

namespace Ui {
    class HierarchyBrowser;
}

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
    void startDrag(Qt::DropActions supportedActions) override { emit dragStarted(supportedActions); }
    void dragEnterEvent(QDragEnterEvent *e) override { emit dragEnter(e); }
    void dragLeaveEvent(QDragLeaveEvent *e) override { emit dragLeave(e); }
    void dragMoveEvent(QDragMoveEvent *e) override { emit dragMove(e); }
    void dropEvent(QDropEvent *e) override { emit drop(e); }
    void paintEvent(QPaintEvent *ev) override;
};

class ObjectsFilter;

class HierarchyBrowser : public QWidget {
    Q_OBJECT

public:
    HierarchyBrowser(QWidget *parent = 0);
    ~HierarchyBrowser();

signals:
    void selected(QList<Object *> objects);
    void focused(Object *object);
    void parented(QList<Object *> objects, Object *parent, int index);
    void updated();
    void removed();
    void menuRequested(Object *object, const QPoint &point);

    void dropMap(QString path, bool additive);

public slots:
    void onObjectSelected(QList<Object *> objects);
    void onSetRootObject(Object *object);
    void onObjectUpdated();

    void onItemRename();

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

private:
    QAction *createAction(const QString &name, const char *member, const QKeySequence &shortcut = 0);

    void changeEvent(QEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::HierarchyBrowser *ui;

    QRubberBand *m_rect;
    QRubberBand *m_line;

    ObjectsFilter *m_filter;

};

#endif // HIERARCHYBROWSER_H
