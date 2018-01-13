#include "objecthierarchymodel.h"

#include <QUrl>
#include <QSize>

#include <aobject.h>

ObjectHierarchyModel::ObjectHierarchyModel(QObject *parent) :
        QAbstractItemModel(parent),
        m_rootItem(nullptr) {
}

void ObjectHierarchyModel::setRoot(AObject *scene) {
    m_rootItem  = scene;
}

AObject *ObjectHierarchyModel::findObject(const QString &ref) {
    QUrl path(ref);
    return m_rootItem->find(path.path().toStdString()); // \todo Review need to check this on errors
}

void ObjectHierarchyModel::reset() {
    beginResetModel();
    endResetModel();
}

QVariant ObjectHierarchyModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }
    AObject *item   = static_cast<AObject* >(index.internalPointer());
    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 0: return QString::fromStdString(item->name());
                case 1: return QString::fromStdString(item->typeName());
                case 2: return item->isEnable();
                default: break;
            }
        } break;
        case Qt::SizeHintRole: {
            if(index.column() == 2) {
                return QSize(16, 1);
            }
        } break;
        case Qt::UserRole: {
            return QString::number(item->uuid());
        } break;
        default: break;
    }
    return QVariant();
}

bool ObjectHierarchyModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    AObject *item   = static_cast<AObject* >(index.internalPointer());
    switch(index.column()) {
        default: {
            item->setName(value.toString().toStdString());
        } break;
    }
    return true;
}

QVariant ObjectHierarchyModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Name");
            case 1: return tr("Class");
            case 2: return tr("");
            default: break;
        }
    }
    return QVariant();
}

int ObjectHierarchyModel::columnCount(const QModelIndex &parent) const {
    return 3;
}

int ObjectHierarchyModel::rowCount(const QModelIndex &parent) const {
    if(m_rootItem) {
        AObject *parentItem = m_rootItem;
        if(parent.isValid()) {
            parentItem      = static_cast<AObject *>(parent.internalPointer());
        }
        return parentItem->getChildren().size();
    }
    return 0;
}

QModelIndex ObjectHierarchyModel::index(int row, int column, const QModelIndex &parent) const {
    if(m_rootItem) {
        AObject *parentItem = m_rootItem;
        if(parent.isValid()) {
            parentItem      = static_cast<AObject *>(parent.internalPointer());
        }
        if(row >= parentItem->getChildren().size() || row < 0) {
            return QModelIndex();
        }
        QList<AObject *> list;
        for(auto it : parentItem->getChildren()) {
            list.push_back(it);
        }
        return createIndex(row, column, list.at(row));
    }
    return QModelIndex();
}

QModelIndex ObjectHierarchyModel::parent(const QModelIndex &index) const {
    if(!index.isValid()) {
        return QModelIndex();
    }

    AObject *childItem  = static_cast<AObject *>(index.internalPointer());
    AObject *parentItem = static_cast<AObject *>(childItem->parent());

    if(!parentItem || parentItem == m_rootItem) {
        return QModelIndex();
    }
    QList<AObject *> list;
    for(auto it : parentItem->parent()->getChildren()) {
        list.push_back(it);
    }
    return createIndex(list.indexOf(parentItem), 0, parentItem);
}

Qt::ItemFlags ObjectHierarchyModel::flags(const QModelIndex &index) const {
    if(!index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    Qt::ItemFlags result    = Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(index.column() == 0) {
        result  |= Qt::ItemIsEditable;
    }
    return result;
}
