#include "baseobjectmodel.h"

BaseObjectModel::BaseObjectModel(QObject *parent) :
        QAbstractItemModel(parent) {

    m_rootItem  = createRoot();
}

QObject *BaseObjectModel::createRoot() {
    return new QObject(this);
}

int BaseObjectModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/ ) const {
    QObject *parentItem = m_rootItem;
    if(parent.isValid()) {
        parentItem      = static_cast<QObject *>(parent.internalPointer());
    }
    return parentItem->children().size();
}

QModelIndex BaseObjectModel::index(int row, int column, const QModelIndex &parent) const {
    QObject *parentItem = m_rootItem;
    if(parent.isValid()) {
        parentItem      = static_cast<QObject *>(parent.internalPointer());
    }
    if(row >= parentItem->children().size() || row < 0) {
        return QModelIndex();
    }
    return createIndex(row, column, parentItem->children().at(row));
}

QModelIndex BaseObjectModel::parent(const QModelIndex &index) const {
    if(!index.isValid()) {
        return QModelIndex();
    }

    QObject *childItem  = static_cast<QObject *>(index.internalPointer());
    QObject *parentItem = childItem->parent();

    if (!parentItem || parentItem == m_rootItem) {
        return QModelIndex();
    }
    return createIndex(parentItem->parent()->children().indexOf(parentItem), 0, parentItem);
}

Qt::ItemFlags BaseObjectModel::flags(const QModelIndex &index) const {
    if(!index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    }
    QObject *item   = static_cast<QObject *>(index.internalPointer());
    // only allow change of value attribute
    if(!item || !item->children().isEmpty()) {
        return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    } else {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
}
