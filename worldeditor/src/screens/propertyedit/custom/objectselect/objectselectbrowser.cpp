#include "objectselectbrowser.h"
#include "ui_objectselectbrowser.h"

#include "screens/objecthierarchy/objecthierarchymodel.h"
#include "assetlist.h"

class AssetFilter : public QSortFilterProxyModel {
public:
    typedef QList<int32_t> TypeList;

    explicit AssetFilter(QObject *parent) :
            QSortFilterProxyModel(parent) {
        sort(0);
    }

    void setContentType(const QString &type) {
        m_type = type;
        invalidate();
    }

protected:
    bool checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
        QString type = sourceModel()->data(index).toString();
        if(type.isEmpty() || m_type == type) {
            return true;
        }
        return false;
    }

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const {
        QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
        return(QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent) &&
                                                       (filterRegularExpression().isValid() || sourceModel()->data(index).toBool()));
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override {
        bool result = true;
        if(!m_type.isEmpty()) {
            result = checkContentTypeFilter(sourceRow, sourceParent);
        }
        result &= checkNameFilter(sourceRow, sourceParent);

        return result;
    }

private:
    QString m_type;

};

ObjectSelectBrowser::ObjectSelectBrowser(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ObjectSelectBrowser),
        m_object(nullptr),
        m_componentProxy(new ObjectsFilter(this)),
        m_contentProxy(new AssetFilter(this)),
        m_contentNone(new ProxyModelNoneEntry(this)) {

    ui->setupUi(this);

    AssetList *assetList = new AssetList;
    m_contentProxy->setSourceModel(assetList);
    m_contentNone->setSourceModel(m_contentProxy);

    ui->listView->setModel(m_contentNone);
    connect(assetList, &AssetList::layoutChanged, this, &ObjectSelectBrowser::onModelUpdated);

    ObjectHierarchyModel *hierarchy = new ObjectHierarchyModel(this);
    hierarchy->showNone();
    m_componentProxy->setSourceModel(hierarchy);

    ui->treeView->setModel(m_componentProxy);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->header()->hideSection(1);
    ui->treeView->header()->hideSection(2);
}

ObjectSelectBrowser::~ObjectSelectBrowser() {
    delete ui;
}

void ObjectSelectBrowser::setTypeFilter(const QString &filter) {
    m_componentProxy->setClassType(filter);
    m_contentProxy->setContentType(filter);
}

void ObjectSelectBrowser::expandToIndex(const QModelIndex &index, QTreeView *view) {
    QModelIndex parent = index.parent();
    if(!parent.isValid()) {
        return;
    }

    if(!view->isExpanded(parent)) {
        view->setExpanded(parent, true);
        expandToIndex(parent, view);
    }
}

void ObjectSelectBrowser::onObjectSelected(Object *object) {
    m_object = object;
    QItemSelectionModel *select = ui->treeView->selectionModel();
    QAbstractItemModel *model = ui->treeView->model();
    select->select(QModelIndex(), QItemSelectionModel::Clear);
    if(m_object) {
        QModelIndexList list = model->match(model->index(0, 0), Qt::UserRole,
                                            QString::number(m_object->uuid()),
                                            -1, Qt::MatchExactly | Qt::MatchRecursive);
        for(QModelIndex &it : list) {
            select->select(it, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            expandToIndex(it, ui->treeView);
        }
    }
    ui->tabWidget->setCurrentIndex(1);
}

void ObjectSelectBrowser::onAssetSelected(const QString &path) {
    m_asset = path;
    AssetList *model = static_cast<AssetList *>(m_contentProxy->sourceModel());
    ui->listView->setCurrentIndex( m_contentNone->mapFromSource(m_contentProxy->mapFromSource(model->findResource(m_asset))) );
    ui->tabWidget->setCurrentIndex(0);
}

void ObjectSelectBrowser::onSetRootObject(Object *object) {
    ObjectHierarchyModel *model = static_cast<ObjectHierarchyModel *>(m_componentProxy->sourceModel());
    model->setRoot(object);

    ui->treeView->expandToDepth(0);
}

void ObjectSelectBrowser::onModelUpdated() {
    onAssetSelected(m_asset);
    m_contentProxy->invalidate();
}

void ObjectSelectBrowser::on_lineEdit_textChanged(const QString &arg1) {
    m_componentProxy->setFilterFixedString(arg1);
    m_contentProxy->setFilterFixedString(arg1);

    onObjectSelected(m_object);
    onAssetSelected(m_asset);
}

void ObjectSelectBrowser::on_treeView_doubleClicked(const QModelIndex &index) {
    QModelIndex origin = m_componentProxy->mapToSource(index);
    emit componentSelected(Engine::findObject(origin.internalId()));
}

void ObjectSelectBrowser::on_listView_doubleClicked(const QModelIndex &index) {
    QModelIndex origin = m_contentProxy->mapToSource(m_contentNone->mapToSource(index));
    AssetList *model = static_cast<AssetList *>(m_contentProxy->sourceModel());
    emit assetSelected(model->path(origin));
}
