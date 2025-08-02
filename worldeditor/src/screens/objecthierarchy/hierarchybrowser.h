#ifndef HIERARCHYBROWSER_H
#define HIERARCHYBROWSER_H

#include <QWidget>
#include <QTreeView>
#include <QMenu>

#include <object.h>
#include <editor/asseteditor.h>
#include <editor/undomanager.h>
#include <editor/editorgadget.h>

namespace Ui {
    class HierarchyBrowser;
}

class ObjectHierarchyModel;
class ComponentBrowser;

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

class HierarchyBrowser : public EditorGadget {
    Q_OBJECT

public:
    HierarchyBrowser(QWidget *parent = 0);
    ~HierarchyBrowser();

public slots:
    void onUpdated() override;

    void onObjectsSelected(const Object::ObjectList &objects) override;
    void onObjectsChanged(const Object::ObjectList &objects, const TString &property, Variant value) override {}

    void onItemRename();

    void onDragEnter(QDragEnterEvent *e);
    void onDragLeave(QDragLeaveEvent *);
    void onDragMove(QDragMoveEvent *e);
    void onDrop(QDropEvent *e);

private slots:
    void onObjectsHierarchyChanged(Object *object);
    void onCreateComponent(const QString &uri);
    void onDragStarted(Qt::DropActions supportedActions);

    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void on_treeView_clicked(const QModelIndex &index);
    void on_treeView_doubleClicked(const QModelIndex &index);
    void on_treeView_customContextMenuRequested(const QPoint &pos);
    void on_lineEdit_textChanged(const QString &arg1);

private:
    void setCurrentEditor(AssetEditor *editor) override;

    QAction *createAction(const QString &name, const char *member, const QKeySequence &shortcut = 0);

    void changeEvent(QEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::HierarchyBrowser *ui;

    AssetEditor *m_currentEditor;

    ComponentBrowser *m_components;

    QRubberBand *m_rect;
    QRubberBand *m_line;

    ObjectsFilter *m_filter;

};

class ParentingObjects : public UndoCommand {
public:
    ParentingObjects(const QList<Object *> &objects, Object *origin, int32_t position, HierarchyBrowser *browser, const QString &name = QObject::tr("Parenting Objects"), QUndoCommand *group = nullptr);
    void undo() override;
    void redo() override;

protected:
    typedef QPair<uint32_t, uint32_t> ParentPair;
    QList<ParentPair> m_dump;
    uint32_t m_parent;
    int32_t m_position;
    std::list<uint32_t> m_objects;

};

#endif // HIERARCHYBROWSER_H
