#include "hierarchybrowser.h"
#include "ui_hierarchybrowser.h"

#include "config.h"

#include "objecthierarchymodel.h"

#include <QDrag>
#include <QPainter>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QStyledItemDelegate>
#include <QWidgetAction>

#include <object.h>
#include <components/scene.h>
#include <components/actor.h>

#include <editor/assetmanager.h>
#include <editor/projectsettings.h>

#include "screens/componentbrowser/componentbrowser.h"

#define ROW_SENCE 4

class HierarchyDelegate : public QStyledItemDelegate {
public:
    explicit HierarchyDelegate(QObject *parent = nullptr) :
            QStyledItemDelegate(parent) {

    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QModelIndex origin = index;
        const ObjectsFilter *filter = dynamic_cast<const ObjectsFilter *>(index.model());
        if(filter) {
            origin = filter->mapToSource(origin);
        }
        if(!index.parent().isValid() && index.column() < 2) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(0, 0, 0, 64));

            QRect r(option.rect);
            r.setLeft(40);
            painter->drawRect(r);
        }
        QStyledItemDelegate::paint(painter, option, index);
    }
};

void TreeView::paintEvent(QPaintEvent *ev) {
    int size = header()->defaultSectionSize();
    int count = header()->count() - 2;
    // Left action pannel
    QPainter painter(viewport());
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 64));
    painter.drawRect(0, 0, size * count, height());

    QTreeView::paintEvent(ev);
}

class RubberBand : public QRubberBand {
public:
    explicit RubberBand(Shape shape, QWidget *parent = nullptr) :
        QRubberBand(shape, parent) {

    }

    void paintEvent(QPaintEvent *ev) {
        if(shape() == Rectangle) {
            QRubberBand::paintEvent(ev);
        } else {
            QPainter p(this);
            p.setPen(QPen(blue, 2));
            p.setBrush(Qt::white);
            p.drawLine(0, 2, geometry().width(), 2);
            p.drawEllipse(1, 1, 3, 3);
        }
    }

    QColor blue = QColor(2, 119, 189); // #0277bd
};

HierarchyBrowser::HierarchyBrowser(QWidget *parent) :
        EditorGadget(parent),
        ui(new Ui::HierarchyBrowser),
        m_currentEditor(nullptr),
        m_components(new ComponentBrowser(this)),
        m_rect(nullptr),
        m_line(nullptr),
        m_filter(new ObjectsFilter(this)) {

    ui->setupUi(this);

    m_rect = new RubberBand(QRubberBand::Rectangle, ui->treeView);
    m_line = new RubberBand(QRubberBand::Line, ui->treeView);

    m_filter->setSourceModel(new ObjectHierarchyModel(this));

    ui->treeView->setModel(m_filter);
    ui->treeView->setItemDelegate(new HierarchyDelegate);
    ui->treeView->installEventFilter(this);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeView->itemDelegate(), SIGNAL(commitData(QWidget*)), this, SIGNAL(updated()));
    connect(ui->treeView, SIGNAL(dragStarted(Qt::DropActions)), this, SLOT(onDragStarted(Qt::DropActions)));
    connect(ui->treeView, SIGNAL(dragEnter(QDragEnterEvent*)), this, SLOT(onDragEnter(QDragEnterEvent*)));
    connect(ui->treeView, SIGNAL(dragMove(QDragMoveEvent*)), this, SLOT(onDragMove(QDragMoveEvent*)));
    connect(ui->treeView, SIGNAL(dragLeave(QDragLeaveEvent*)), this, SLOT(onDragLeave(QDragLeaveEvent*)));
    connect(ui->treeView, SIGNAL(drop(QDropEvent*)), this, SLOT(onDrop(QDropEvent*)));

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &HierarchyBrowser::onSelectionChanged);

    ui->treeView->header()->moveSection(2, 0);
    ui->treeView->header()->moveSection(3, 1);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->treeView->header()->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->treeView->header()->hideSection(1);

    ui->toolButton->setProperty("blue", true);

    QMenu *menu = new QMenu(ui->toolButton);
    QWidgetAction *action = new QWidgetAction(menu);
    action->setDefaultWidget(m_components);
    menu->addAction(action);
    ui->toolButton->setMenu(menu);

    connect(m_components, &ComponentBrowser::componentSelected, this, &HierarchyBrowser::onCreateComponent);
    connect(m_components, &ComponentBrowser::componentSelected, menu, &QMenu::hide);
}

HierarchyBrowser::~HierarchyBrowser() {
    delete ui;
}

void HierarchyBrowser::setCurrentEditor(AssetEditor *editor) {
    if(m_currentEditor) {
        disconnect(m_currentEditor, &AssetEditor::objectsHierarchyChanged, this, &HierarchyBrowser::onObjectsHierarchyChanged);
    }

    m_currentEditor = editor;
    if(m_currentEditor) {
        m_components->setGroups(m_currentEditor->componentGroups());
    }

    onObjectsHierarchyChanged(nullptr);

    connect(m_currentEditor, &AssetEditor::objectsHierarchyChanged, this, &HierarchyBrowser::onObjectsHierarchyChanged, Qt::DirectConnection);
}

void HierarchyBrowser::onObjectsHierarchyChanged(Object *object) {
    ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(m_filter->sourceModel());
    model->setRoot(object);

    ui->toolButton->setVisible(object != nullptr);

    ui->treeView->expandToDepth(0);
}

void expandToIndex(const QModelIndex &index, QTreeView *view) {
    QModelIndex parent = index.parent();
    if(!parent.isValid()) {
        return;
    }

    if(!view->isExpanded(parent)) {
        view->setExpanded(parent, true);
        expandToIndex(parent, view);
    }
}

void HierarchyBrowser::onObjectsSelected(QList<Object *> objects) {
    blockSignals(true);
    QItemSelectionModel *select = ui->treeView->selectionModel();
    QAbstractItemModel *model = ui->treeView->model();
    select->select(QModelIndex(), QItemSelectionModel::Clear);
    for(auto object : objects) {
        QModelIndexList list = model->match(model->index(0, 0), Qt::UserRole,
                                            QString::number(object->uuid()),
                                            -1, Qt::MatchExactly | Qt::MatchRecursive);
        for(QModelIndex &it : list) {
            select->select(it, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            expandToIndex(it, ui->treeView);
        }
    }
    blockSignals(false);
}

void HierarchyBrowser::onUpdated() {
    QAbstractItemModel *model = m_filter->sourceModel();

    emit model->layoutAboutToBeChanged();
    emit model->layoutChanged();
}

void HierarchyBrowser::onCreateComponent(const QString &type) {
    if(m_currentEditor) {
        m_currentEditor->onObjectCreate(type);
    }
}

void HierarchyBrowser::onDragEnter(QDragEnterEvent *e) {
    if(e->mimeData()->hasFormat(gMimeObject)) {
        e->acceptProposedAction();
        return;
    } else if(e->mimeData()->hasFormat(gMimeContent)) {
        if(m_currentEditor) {
            m_currentEditor->onDragEnter(e);
        }

        if(e->isAccepted()) {
            return;
        }
    }
    e->ignore();
}

void HierarchyBrowser::onDragLeave(QDragLeaveEvent *) {
    m_rect->hide();
    m_line->hide();
}

void HierarchyBrowser::onDragMove(QDragMoveEvent *e) {
    QModelIndex index = ui->treeView->indexAt(e->pos());
    if(index.isValid()) {
        QRect r = ui->treeView->visualRect(index);
        r.setWidth(ui->treeView->width());
        r.translate(0, ui->treeView->header()->height() + 1);

        int y = ui->treeView->header()->height() + 1 + e->pos().y();
        if(abs(r.top() - y) < ROW_SENCE && index.parent().isValid()) { // Before
            r.setBottom(r.top() + 1);
            r.setTop(r.top() - 3);

            m_rect->hide();
            m_line->show();
            m_line->setGeometry(r);
            m_line->repaint();
        } else if(abs(r.bottom() - y) < ROW_SENCE && index.parent().isValid()) { // After
            QModelIndex child = ui->treeView->model()->index(0, 0, index);
            if(child.isValid()) {
                QRect c = ui->treeView->visualRect(child);
                r.setX(c.x());
            }

            r.setTop(r.bottom() - 2);
            r.setBottom(r.bottom() + 2);

            m_rect->hide();
            m_line->show();
            m_line->setGeometry(r);
            m_line->repaint();
        } else {
            r.setX(0);

            m_line->hide();
            m_rect->show();
            m_rect->setGeometry(r);
            m_rect->repaint();
        }
    } else {
        m_rect->hide();
        m_line->hide();
    }
}

void HierarchyBrowser::onDrop(QDropEvent *e) {
    ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(m_filter->sourceModel());

    if(e->mimeData()->hasFormat(gMimeObject)) {
        Object *parent = model->root();
        int position = -1;

        QList<Object *> objects;

        QString path(e->mimeData()->data(gMimeObject));
        foreach(const QString &it, path.split(";")) {
            QString id = it.left(it.indexOf(':'));
            Object *item = Engine::findObject(id.toUInt());
            if(item) {
                QModelIndex origin = ui->treeView->indexAt(e->pos());
                QModelIndex index = m_filter->mapToSource(origin);
                if(index.isValid()) {
                    QRect r = ui->treeView->visualRect(origin);
                    r.setWidth(ui->treeView->width());
                    r.translate(0, ui->treeView->header()->height() + 1);

                    int y = ui->treeView->header()->height() + 1 + e->pos().y();
                    if(abs(r.top() - y) < ROW_SENCE && index.parent().isValid()) {
                        // Set before
                        position = index.row();
                        index = index.parent();
                    } else if(abs(r.bottom() - y) < ROW_SENCE && index.parent().isValid()) {
                        // Set after
                        position = index.row() + 1;
                        index = index.parent();
                    }

                    Object *indexObject = Engine::findObject(index.internalId());
                    if(item != indexObject) {
                        objects.push_back(item);
                        parent = indexObject;
                    }
                }
            }
        }

        if(!objects.empty()) {
            UndoManager::instance()->push(new ParentingObjects(objects, parent, position, this));
        }
    } else if(e->mimeData()->hasFormat(gMimeContent)) {
        if(m_currentEditor) {
            QDropEvent ev(e->pos(), e->dropAction(), e->mimeData(), e->mouseButtons(), Qt::ControlModifier);
            m_currentEditor->onDrop(&ev);
        }
    }

    m_rect->hide();
    m_line->hide();
}

void HierarchyBrowser::onDragStarted(Qt::DropActions supportedActions) {
    ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(m_filter->sourceModel());

    QMimeData *mimeData = new QMimeData;
    QStringList list;
    foreach(const QModelIndex &it, ui->treeView->selectionModel()->selectedIndexes()) {
        QModelIndex index = m_filter->mapToSource(it);
        if(index.column() == 0) {
            Object *object = Engine::findObject(index.internalId());
            list.push_back(QString::number(object->uuid()) + ":" + object->name().c_str());
        }
    }
    mimeData->setData(gMimeObject, qPrintable(list.join(";")));

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(supportedActions, Qt::MoveAction);
}

void HierarchyBrowser::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    if(selected.indexes().isEmpty()) {
        emit objectsSelected({}, false);
    }
}

void HierarchyBrowser::on_treeView_clicked(const QModelIndex &index) {
    ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(m_filter->sourceModel());

    Object *object = Engine::findObject(m_filter->mapToSource(index).internalId());
    Actor *actor = dynamic_cast<Actor *>(object);
    if(actor) {
        if(index.column() == 0) {
            QSet<Object *> set;

            QItemSelectionModel *selectionModel = ui->treeView->selectionModel();
            for(auto it : selectionModel->selectedIndexes()) {
                Object *object = Engine::findObject(m_filter->mapToSource(it).internalId());

                set.insert(object);
            }

            emit objectsSelected(QList<Object *>(set.begin(), set.end()), false);
        } else if(index.column() == 2) {
            actor->setHideFlags(actor->hideFlags() ^ Actor::ENABLE);
            onUpdated();
        } else if(index.column() == 3) {
            actor->setHideFlags(actor->hideFlags() ^ Actor::SELECTABLE);
            onUpdated();
        }
    }
}

void HierarchyBrowser::on_treeView_doubleClicked(const QModelIndex &index) {
    ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(m_filter->sourceModel());

    Object *object = Engine::findObject(m_filter->mapToSource(index).internalId());

    emit objectsSelected({object}, true);
}

void HierarchyBrowser::on_lineEdit_textChanged(const QString &arg1) {
    m_filter->setFilterFixedString(arg1);
}

void HierarchyBrowser::on_treeView_customContextMenuRequested(const QPoint &pos) {
    ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(m_filter->sourceModel());

    QItemSelectionModel *select = ui->treeView->selectionModel();

    QList<Object *> list;
    foreach(QModelIndex it, select->selectedRows()) {
        Object *object = Engine::findObject(m_filter->mapToSource(it).internalId());
        Actor *actor = dynamic_cast<Actor *>(object);
        if(actor) {
            list.push_back(actor);
        }
    }
    if(!list.empty()) {
        emit objectsSelected(list, false);
    }

    QPoint point = static_cast<QWidget*>(QObject::sender())->mapToGlobal(pos);

    QModelIndex index = ui->treeView->indexAt(pos);
    Object *object = Engine::findObject(m_filter->mapToSource(index).internalId());

    if(m_currentEditor) {
        QMenu *menu = m_currentEditor->objectContextMenu(object);
        if(menu) {
            foreach(QAction *action, menu->actions()) {
                if(action->shortcut() == QKeySequence(Qt::Key_F2)) {
                    connect(action, &QAction::triggered, this, &HierarchyBrowser::onItemRename, Qt::UniqueConnection);
                }
            }

            menu->exec(point);
        }
    }
}

void HierarchyBrowser::onItemRename() {
    QItemSelectionModel *select = ui->treeView->selectionModel();
    foreach(QModelIndex it, select->selectedRows()) {
        ui->treeView->edit(it);
    }
}

void HierarchyBrowser::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

bool HierarchyBrowser::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch(keyEvent->key()) {
            case Qt::Key_Delete: {
                if(m_currentEditor) {
                    ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(m_filter->sourceModel());

                    QList<Object *> list;
                    QItemSelectionModel *select = ui->treeView->selectionModel();
                    foreach(QModelIndex it, select->selectedRows()) {
                        list.push_back(Engine::findObject(m_filter->mapToSource(it).internalId()));
                    }

                    m_currentEditor->onObjectsDeleted(list);
                }
            } break;
            case Qt::Key_F2: {
                onItemRename();
            } break;
        }
        return true;
    }
    return QObject::eventFilter(obj, event);
}

ParentingObjects::ParentingObjects(const QList<Object *> &objects, Object *origin, int32_t position, HierarchyBrowser *browser, const QString &name, QUndoCommand *group) :
        UndoCommand(name, browser, group) {
    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }

    m_parent = origin->uuid();
    m_position = position;
}
void ParentingObjects::undo() {
    HierarchyBrowser *browser = static_cast<HierarchyBrowser *>(m_editor);

    auto ref = m_dump.begin();
    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            if(object->uuid() == ref->first) {
                object->setParent(Engine::findObject(ref->second));
            }
        }
        ++ref;
    }

    browser->onUpdated();
}
void ParentingObjects::redo() {
    HierarchyBrowser *browser = static_cast<HierarchyBrowser *>(m_editor);

    m_dump.clear();
    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            ParentPair pair;
            pair.first =  object->uuid();
            pair.second = object->parent()->uuid();
            m_dump.push_back(pair);

            Object *parent = Engine::findObject(m_parent);
            object->setParent(parent, m_position);
        }
    }

    browser->onUpdated();
}
