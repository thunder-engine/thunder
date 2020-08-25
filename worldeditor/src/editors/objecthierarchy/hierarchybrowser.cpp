#include "hierarchybrowser.h"
#include "ui_hierarchybrowser.h"

#include <QSortFilterProxyModel>
#include <QDrag>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QAction>
#include <QStyledItemDelegate>

#include <object.h>
#include <invalid.h>
#include <components/actor.h>
#include <components/component.h>

#include "config.h"

#include "objecthierarchymodel.h"
#include "controllers/objectctrl.h"
#include "develop/managers/undomanager/undomanager.h"

class ObjectsFilter : public QSortFilterProxyModel {
public:
    explicit ObjectsFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {
        m_HideComponents = false;
    }

    void setClassTypes(const QStringList &list) {
        m_List = list;
        invalidate();
    }

    void setHideComponents(bool hide) {
        m_HideComponents = hide;
        invalidate();
    }

protected:
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) {
        QSortFilterProxyModel::sort(column, order);
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        bool result = true;

        Object *object = static_cast<Object *>(sourceModel()->index(sourceRow, 1, sourceParent).internalPointer());

        if(!m_List.isEmpty()) {
            result &= checkClassTypeFilter(sourceRow, sourceParent);
        }
        if(m_HideComponents) {
            result &= (dynamic_cast<Component*>(object) == nullptr);
            result &= (dynamic_cast<Invalid*>(object) == nullptr);
        }
        result &= checkNameFilter(sourceRow, sourceParent);

        return result;
    }

    bool checkClassTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QAbstractItemModel *model = sourceModel();
        QModelIndex index = model->index(sourceRow, 1, sourceParent);
        QString type = sourceModel()->data(index).toString();
        if(!m_List.contains(type)) {
            for(int i = 0; i < model->rowCount(index); i++) {
                if(checkClassTypeFilter(i, index)) {
                    return true;
                }
            }
            return false;
        }
        return true;
    }

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QAbstractItemModel *model = sourceModel();
        QModelIndex index = model->index(sourceRow, 0, sourceParent);
        if(!filterRegExp().isEmpty() && index.isValid()) {
            for(int i = 0; i < model->rowCount(index); i++) {
                if(checkNameFilter(i, index)) {
                    return true;
                }
            }
            QString key = model->data(index, filterRole()).toString();
            return key.contains(filterRegExp());
        }
        return true;
    }

    QStringList m_List;
    bool m_HideComponents;
};

class HierarchyDelegate : public QStyledItemDelegate {
public:
    explicit HierarchyDelegate(QObject *parent = nullptr) :
            QStyledItemDelegate(parent) {

    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override {
        Q_UNUSED(index)
        editor->setGeometry( option.rect );
    }
};

HierarchyBrowser::HierarchyBrowser(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::HierarchyBrowser),
        m_pRect(nullptr),
        m_pFilter(nullptr),
        m_pController(nullptr) {

    ui->setupUi(this);

    m_pRect = new QRubberBand(QRubberBand::Rectangle, ui->treeView);

    m_pFilter = new ObjectsFilter(this);
    m_pFilter->setSourceModel(new ObjectHierarchyModel(this));
    m_pFilter->setHideComponents(true);

    ui->treeView->setModel(m_pFilter);
    ui->treeView->setItemDelegate(new HierarchyDelegate);
    ui->treeView->installEventFilter(this);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeView->itemDelegate(), SIGNAL(commitData(QWidget *)), this, SIGNAL(updated()));
    connect(ui->treeView, SIGNAL(dragStarted(Qt::DropActions)), this, SLOT(onDragStarted(Qt::DropActions)));
    connect(ui->treeView, SIGNAL(dragEnter(QDragEnterEvent *)), this, SLOT(onDragEnter(QDragEnterEvent *)));
    connect(ui->treeView, SIGNAL(dragMove(QDragMoveEvent *)), this, SLOT(onDragMove(QDragMoveEvent *)));
    connect(ui->treeView, SIGNAL(dragLeave(QDragLeaveEvent*)), this, SLOT(onDragLeave(QDragLeaveEvent*)));
    connect(ui->treeView, SIGNAL(drop(QDropEvent*)), this, SLOT(onDrop(QDropEvent *)));

    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->treeView->header()->resizeSection(1, 50);
    ui->treeView->header()->hideSection(1);
    ui->treeView->header()->hideSection(3);

    m_ContentMenu.addAction(createAction(tr("Rename"), SLOT(onItemRename()), QKeySequence(Qt::Key_F2)));
    m_ContentMenu.addAction(createAction(tr("Duplicate"), SLOT(onItemDuplicate())));
    m_ContentMenu.addAction(createAction(tr("Delete"), SLOT(onItemDelete()), QKeySequence(Qt::Key_Delete)));
    m_ContentMenu.addSeparator();

    m_Prefab.push_back((createAction(tr("Unpack Prefab"), SLOT(onItemUnpack()))));
    m_Prefab.push_back(createAction(tr("Unpack Prefab Completely"), SLOT(onItemUnpackAll())));

    for(auto it : m_Prefab) {
        m_ContentMenu.addAction(it);
    }
}

HierarchyBrowser::~HierarchyBrowser() {
    delete ui;
}

void HierarchyBrowser::setRootObject(Object *object) {
    static_cast<ObjectHierarchyModel *>(m_pFilter->sourceModel())->setRoot(object);
    onHierarchyUpdated();
}

void HierarchyBrowser::setSimplified(bool enable) {
    ui->treeView->header()->setSectionHidden(2, enable);
    ui->treeView->setDragEnabled(!enable);
}

void HierarchyBrowser::setComponentsFilter(const QStringList &list) {
    m_pFilter->setClassTypes(list);
}

void HierarchyBrowser::setController(ObjectCtrl *ctrl) {
    m_pController = ctrl;
}

void HierarchyBrowser::onObjectSelected(Object::ObjectList objects) {
    QItemSelectionModel *select = ui->treeView->selectionModel();
    QAbstractItemModel *model = ui->treeView->model();
    select->select(QModelIndex(), QItemSelectionModel::Clear);
    for(auto object : objects) {
        QModelIndexList list = model->match(model->index(0, 0), Qt::UserRole,
                                            QString::number(object->uuid()),
                                            -1, Qt::MatchExactly | Qt::MatchRecursive);
        foreach(const QModelIndex &it, list) {
            select->select(it, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}

void HierarchyBrowser::onHierarchyUpdated() {
    static_cast<ObjectHierarchyModel *>(m_pFilter->sourceModel())->reset();
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
    QRect r;
    QModelIndex index = ui->treeView->indexAt(e->pos());
    if(index.isValid()) {
        r = ui->treeView->visualRect(index);
        r.setX(0);
        r.setWidth(ui->treeView->width());
        r.translate(0, ui->treeView->header()->height() + 1);
    } else {
        r = ui->treeView->rect();
        r.setTop(ui->treeView->header()->rect().bottom());
    }
    m_pRect->show();
    m_pRect->setGeometry(r);
    m_pRect->repaint();
}

void HierarchyBrowser::onDrop(QDropEvent *e) {
    ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(m_pFilter->sourceModel());

    Object::ObjectList objects;
    Object *parent = model->root();
    if(e->mimeData()->hasFormat(gMimeObject)) {
        QString path(e->mimeData()->data(gMimeObject));
        foreach(const QString &it, path.split(";")) {
            QString id = it.left(it.indexOf(':'));
            Object *item = model->findObject(id.toUInt());
            QModelIndex index = m_pFilter->mapToSource(ui->treeView->indexAt(e->pos()));
            if(item) {
                objects.push_back(item);
                if(index.isValid()) {
                    if(item != index.internalPointer()) {
                        parent = static_cast<Object *>(index.internalPointer());
                    }
                }
            }
        }
    }
    if(!objects.empty()) {
        emit parented(objects, parent);
    }
    m_pRect->hide();
}

void HierarchyBrowser::onDragStarted(Qt::DropActions supportedActions) {
    QMimeData *mimeData = new QMimeData;
    QStringList list;
    foreach(const QModelIndex &it, ui->treeView->selectionModel()->selectedIndexes()) {
        QModelIndex index = m_pFilter->mapToSource(it);
        if(index.column() == 0) {
            Object *object = static_cast<Object *>(index.internalPointer());
            list.push_back(QString::number(object->uuid()) + ":" + object->name().c_str());
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
        object->setEnabled(!object->isEnabled());
    }
    emit selected(list);
}

void HierarchyBrowser::on_treeView_doubleClicked(const QModelIndex &index) {
    emit focused(static_cast<Object *>(m_pFilter->mapToSource(index).internalPointer()));
}

void HierarchyBrowser::on_lineEdit_textChanged(const QString &arg1) {
    m_pFilter->setFilterFixedString(arg1);
}

void HierarchyBrowser::on_treeView_customContextMenuRequested(const QPoint &pos) {
    QWidget *w = static_cast<QWidget*>(QObject::sender());
    QItemSelectionModel *select = ui->treeView->selectionModel();
    if(!select->selectedIndexes().empty()) {
        bool enabled = false;
        foreach(QModelIndex it, select->selectedRows()) {
            Object *object = static_cast<Object *>(m_pFilter->mapToSource(it).internalPointer());
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor && actor->isInstance()) {
                enabled = true;
            }
        }

        for(auto it : m_Prefab) {
            it->setEnabled(enabled);
        }

        m_ContentMenu.exec(w->mapToGlobal(pos));
    }
}

void HierarchyBrowser::onItemRename() {
    QItemSelectionModel *select = ui->treeView->selectionModel();
    foreach(QModelIndex it, select->selectedRows()) {
        ui->treeView->edit(it);
    }
}

void HierarchyBrowser::onItemDuplicate() {
    UndoManager::instance()->push(new DuplicateObjects(m_pController));
}

void HierarchyBrowser::onItemDelete() {
    QItemSelectionModel *select = ui->treeView->selectionModel();
    Object::ObjectList list;
    foreach(QModelIndex it, select->selectedRows()) {
        list.push_back(static_cast<Object *>(m_pFilter->mapToSource(it).internalPointer()));
    }
    emit removed(list);
}

void HierarchyBrowser::onItemUnpack() {
    QItemSelectionModel *select = ui->treeView->selectionModel();
    foreach(QModelIndex it, select->selectedRows()) {
        Object *object = static_cast<Object *>(m_pFilter->mapToSource(it).internalPointer());
        Actor *actor = dynamic_cast<Actor *>(object);
        if(actor) {
            actor->setPrefab(nullptr);
        }
    }
}

void unpackHelper(Object *object) {
    if(object) {
        Actor *actor = dynamic_cast<Actor *>(object);
        if(actor) {
            actor->setPrefab(nullptr);
        }

        for(auto it : object->getChildren()) {
            unpackHelper(it);
        }
    }
}

void HierarchyBrowser::onItemUnpackAll() {
    QItemSelectionModel *select = ui->treeView->selectionModel();
    foreach(QModelIndex it, select->selectedRows()) {
        unpackHelper(static_cast<Object *>(m_pFilter->mapToSource(it).internalPointer()));
    }
}

QAction *HierarchyBrowser::createAction(const QString &name, const char *member, const QKeySequence &shortcut) {
    QAction *a = new QAction(name, this);
    a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    a->setShortcut(shortcut);
    connect(a, SIGNAL(triggered(bool)), this, member);
    ui->treeView->addAction(a);
    return a;
}
