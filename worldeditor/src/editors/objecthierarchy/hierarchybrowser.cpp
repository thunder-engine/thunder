#include "hierarchybrowser.h"
#include "ui_hierarchybrowser.h"

#include <QSortFilterProxyModel>
#include <QDrag>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QAction>

#include <object.h>
#include <invalid.h>
#include <components/actor.h>
#include <components/component.h>

#include "config.h"

#include "objecthierarchymodel.h"
#include "managers/undomanager/undomanager.h"

class ObjectsFilter : public QSortFilterProxyModel {
public:
    explicit ObjectsFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {
        m_HideComponents    = false;
    }

    void setClassTypes(const QStringList &list) {
        m_List      = list;
        invalidate();
    }

    void setHideComponents(bool hide) {
        m_HideComponents    = hide;
        invalidate();
    }

protected:
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) {
        QSortFilterProxyModel::sort(column, order);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        bool result = true;
        if(m_HideComponents) {
            Object *object  = static_cast<Object *>(sourceModel()->index(sourceRow, 1, sourceParent).internalPointer());
            result &= (dynamic_cast<Component*>(object) == nullptr);
            result &= (dynamic_cast<Invalid*>(object) == nullptr);
        }
        if(!m_List.isEmpty()) {
            result &= checkClassTypeFilter(sourceRow, sourceParent);
        }
        result     &= checkNameFilter(sourceRow, sourceParent);

        return result;
    }

    bool checkClassTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QModelIndex index   = sourceModel()->index(sourceRow, 1, sourceParent);
        foreach (QString it, m_List) {
            QString type    = sourceModel()->data(index).toString();
            if(it == type) {
                return true;
            }
        }
        return false;
    }

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QAbstractItemModel *model   = sourceModel();
        QModelIndex index           = model->index(sourceRow, 0, sourceParent);
        if(!filterRegExp().isEmpty() && index.isValid()) {
            for(int i = 0; i < model->rowCount(index); i++) {
                if(checkNameFilter(i, index)) {
                    return true;
                }
            }
            QString key = model->data(index, filterRole()).toString();
            return key.contains(filterRegExp());
        }
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }

    QStringList     m_List;
    bool            m_HideComponents;
};

HierarchyBrowser::HierarchyBrowser(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::HierarchyBrowser) {
        ui->setupUi(this);

    m_pRect = new QRubberBand(QRubberBand::Rectangle, ui->treeView);

    m_pFilter   = new ObjectsFilter(this);
    m_pFilter->setSourceModel(new ObjectHierarchyModel(this));
    m_pFilter->setHideComponents(true);

    ui->treeView->setModel(m_pFilter);
    ui->treeView->installEventFilter(this);

    connect(ui->treeView->itemDelegate(), SIGNAL(commitData(QWidget *)), this, SIGNAL(updated()));
    connect(ui->treeView, SIGNAL(dragStarted(Qt::DropActions)), this, SLOT(onDragStarted(Qt::DropActions)));
    connect(ui->treeView, SIGNAL(dragEnter(QDragEnterEvent *)), this, SLOT(onDragEnter(QDragEnterEvent *)));
    connect(ui->treeView, SIGNAL(dragMove(QDragMoveEvent *)), this, SLOT(onDragMove(QDragMoveEvent *)));
    connect(ui->treeView, SIGNAL(dragLeave(QDragLeaveEvent*)), this, SLOT(onDragLeave(QDragLeaveEvent*)));
    connect(ui->treeView, SIGNAL(drop(QDropEvent*)), this, SLOT(onDrop(QDropEvent *)));

    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->treeView->header()->moveSection(2, 0);
    ui->treeView->header()->hideSection(1);
    ui->treeView->header()->hideSection(3);

    createAction(tr("Duplicate"), SLOT(onItemDuplicate()));
    createAction(tr("Rename"), SLOT(onItemRename()), QKeySequence(Qt::Key_F2));
    createAction(tr("Delete"), SLOT(onItemDelete()), QKeySequence(Qt::Key_Delete));
}

HierarchyBrowser::~HierarchyBrowser() {
    delete ui;
}

void HierarchyBrowser::setObject(Object *object) {
    static_cast<ObjectHierarchyModel *>(m_pFilter->sourceModel())->setRoot(object);
    onHierarchyUpdated();
}

void HierarchyBrowser::onObjectSelected(Object::ObjectList objects) {
    QItemSelectionModel *select = ui->treeView->selectionModel();
    QAbstractItemModel *model   = ui->treeView->model();
    select->select(QModelIndex(), QItemSelectionModel::Clear);
    for(auto object : objects) {
        QModelIndexList list    = model->match(model->index(0, 0), Qt::UserRole,
                                               QString::number(object->uuid()),
                                               -1, Qt::MatchExactly | Qt::MatchRecursive);
        foreach(const QModelIndex &it, list) {
            select->select(it, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}

void HierarchyBrowser::onHierarchyUpdated() {
    static_cast<ObjectHierarchyModel *>(m_pFilter->sourceModel())->reset();
    ui->treeView->expandAll();
}

void HierarchyBrowser::onDragEnter(QDragEnterEvent *e) {
    if(e->mimeData()->hasFormat(gMimeObject)) {
        e->acceptProposedAction();
        return;
    }
    e->ignore();
}

void HierarchyBrowser::onDragLeave(QDragLeaveEvent *) {
    m_pRect->hide();
}

void HierarchyBrowser::onDragMove(QDragMoveEvent *e) {
    QModelIndex index   = ui->treeView->indexAt(e->pos());
    QRect r;
    if(index.isValid()) {
        r   = ui->treeView->visualRect(index);
        r.setX(0);
        r.setWidth(ui->treeView->width());
        r.translate(0, ui->treeView->header()->height() + 1);
    } else {
        r   = ui->treeView->rect();
        r.setTop(ui->treeView->header()->rect().bottom());
    }
    m_pRect->show();
    m_pRect->setGeometry(r);
    m_pRect->repaint();
}

void HierarchyBrowser::onDrop(QDropEvent *e) {
    Object::ObjectList objects;
    Object::ObjectList parents;
    if(e->mimeData()->hasFormat(gMimeObject)) {
        QString path(e->mimeData()->data(gMimeObject));
        foreach(const QString &it, path.split(";")) {
            ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(m_pFilter->sourceModel());
            Object *item    = model->findObject(it.toUInt());
            QModelIndex index   = m_pFilter->mapToSource(ui->treeView->indexAt(e->pos()));
            if(item) {
                objects.push_back(item);
                if(index.isValid()) {
                    if(item != index.internalPointer()) {
                        parents.push_back(static_cast<Object *>(index.internalPointer()));
                    }
                } else {
                    parents.push_back(model->root());
                }
            }
        }
    }
    if(!objects.empty() && objects.size() == parents.size()) {
        emit parented(objects, parents);
    }
    m_pRect->hide();
}

void HierarchyBrowser::onDragStarted(Qt::DropActions supportedActions) {
    QMimeData *mimeData = new QMimeData;
    QStringList list;
    foreach(const QModelIndex &it, ui->treeView->selectionModel()->selectedIndexes()) {
        QModelIndex index   = m_pFilter->mapToSource(it);
        if(index.column() == 0) {
            Object *object  = static_cast<Object *>(index.internalPointer());
            list.push_back(QString::number(object->uuid()));
        }
    }
    mimeData->setData(gMimeObject, qPrintable(list.join(";")));

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(supportedActions, Qt::MoveAction);
}

void HierarchyBrowser::on_treeView_clicked(const QModelIndex &index) {
    QItemSelectionModel *select = ui->treeView->selectionModel();
    Object::ObjectList list;
    foreach(QModelIndex it, select->selectedRows()) {
        list.push_back(static_cast<Object *>(m_pFilter->mapToSource(it).internalPointer()));
    }

    if(index.column() == 2) {
        Actor *object  = static_cast<Actor *>(m_pFilter->mapToSource(index).internalPointer());
        object->setEnable(!object->isEnable());
    }
    emit selected(list);
}

void HierarchyBrowser::on_treeView_doubleClicked(const QModelIndex &index) {
    emit focused(static_cast<Object *>(m_pFilter->mapToSource(index).internalPointer()));
}

void HierarchyBrowser::on_lineEdit_textChanged(const QString &arg1) {
    m_pFilter->setFilterFixedString(arg1);
}

void HierarchyBrowser::onItemRename() {

}

void HierarchyBrowser::onItemDuplicate() {

}

void HierarchyBrowser::onItemDelete() {
    QItemSelectionModel *select = ui->treeView->selectionModel();
    Object::ObjectList list;
    foreach(QModelIndex it, select->selectedRows()) {
        list.push_back(static_cast<Object *>(m_pFilter->mapToSource(it).internalPointer()));
    }
    emit removed(list);
}

void HierarchyBrowser::createAction(const QString &name, const char *member, const QKeySequence &shortcut) {
    QAction *a  = new QAction(name, this);
    a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    a->setShortcut(shortcut);
    connect(a, SIGNAL(triggered(bool)), this, member);
    ui->treeView->addAction(a);
}
