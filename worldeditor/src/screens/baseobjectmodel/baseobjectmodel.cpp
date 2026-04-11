#include "baseobjectmodel.h"

BaseObjectModel::BaseObjectModel(QObject *parent) :
        QAbstractItemModel(parent) {

    m_rootItem = createRoot();
}

QObject *BaseObjectModel::createRoot() {
    QObject *root = new QObject(this);
    addItem(root);
    return root;
}

int BaseObjectModel::rowCount(const QModelIndex &parent) const {
    QObject *parentItem = m_rootItem;
    if(parent.isValid()) {
        parentItem = getObject(parent);
    }
    if(parentItem) {
        return parentItem->children().size();
    }
    return 0;
}

QModelIndex BaseObjectModel::index(int row, int column, const QModelIndex &parent) const {
    QObject *parentItem = m_rootItem;
    if(parent.isValid()) {
        parentItem = getObject(parent);
    }
    if(!parentItem || row >= parentItem->children().size() || row < 0) {
        return QModelIndex();
    }

    QObject *item = parentItem->children().at(row);
    return createIndex(row, column, reinterpret_cast<quintptr>(item));
}

QModelIndex BaseObjectModel::parent(const QModelIndex &index) const {
    if(!index.isValid()) {
        return QModelIndex();
    }

    QObject *childItem = getObject(index);
    if(childItem) {
        QObject *parentItem = childItem->parent();
        if(!parentItem || parentItem == m_rootItem) {
            return QModelIndex();
        }

        QObject *superParent = parentItem->parent();
        if(!superParent) {
            return QModelIndex();
        }
        int row = superParent->children().indexOf(parentItem);
        return createIndex(row, 0, reinterpret_cast<quintptr>(parentItem));
    }
    return QModelIndex();
}

Qt::ItemFlags BaseObjectModel::flags(const QModelIndex &index) const {
    if(!index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    }
    QObject *item = getObject(index);
    // only allow change of value attribute
    if(!item || !item->children().isEmpty()) {
        return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    }
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex BaseObjectModel::getIndex(QObject *object, const QModelIndex &parent) const {
    for(int i = 0; i < rowCount(parent); i++) {
        QModelIndex index = BaseObjectModel::index(i, 0, parent);
        if(getObject(index) == object) {
            return index;
        }
        index = getIndex(object, index);
        if(index.isValid()) {
            return index;
        }
    }
    return QModelIndex();
}

bool BaseObjectModel::removeResource(const QModelIndex &index) {
    return m_items.remove(index.internalId());
}

void BaseObjectModel::addItem(QObject *object) {
    m_items[reinterpret_cast<quintptr>(object)] = object;
}

QObject *BaseObjectModel::getObject(const QModelIndex &index) const {
    quintptr id = index.internalId();
    return m_items.value(id, nullptr);
}

void BaseObjectModel::clear() {
    foreach(QObject *it, m_rootItem->children()) {
        it->setParent(nullptr);
        it->deleteLater();
    }
    m_items.clear();
}

