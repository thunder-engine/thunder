#include "objecthierarchymodel.h"

#include <QUrl>
#include <QSize>

#include "components/actor.h"

ObjectHierarchyModel::ObjectHierarchyModel(QObject *parent) :
        QAbstractItemModel(parent),
        m_rootItem(nullptr),
        m_Visible(":/Style/styles/dark/icons/eye.png"),
        m_Invisible(":/Style/styles/dark/icons/eye_close.png"),
        m_Prefab(":/Images/editor/objects/prefab.png"),
        m_Actor(":/Images/editor/objects/actor.png") {

}

void ObjectHierarchyModel::setRoot(Object *scene) {
    m_rootItem = scene;
}

Object *ObjectHierarchyModel::findObject(const uint32_t uuid, Object *parent) {
    Object *result = nullptr;
    if(parent == nullptr) {
        parent = m_rootItem;
    }
    for(Object *it : parent->getChildren()) {
        if(it->uuid() == uuid) {
            return it;
        } else {
            result = findObject(uuid, it);
            if(result != nullptr) {
                return result;
            }
        }
    }

    return result;
}

void ObjectHierarchyModel::reset() {
    beginResetModel();
    endResetModel();
}

QVariant ObjectHierarchyModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }
    Actor *item = static_cast<Actor* >(index.internalPointer());

    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            switch(index.column()) {
                case 0: return QString::fromStdString(item->name());
                case 1: return QString::fromStdString(item->typeName());
                default: break;
            }
        } break;
        case Qt::DecorationRole: {
            if(index.column() == 0) {
                return item->isInstance() ? m_Prefab : m_Actor;
            }
            if(index.column() == 2) {
                return item->isEnabled() ? m_Visible : m_Invisible;
            }
        } break;
        case Qt::TextColorRole: {
            if(item->isInstance()) {
                if(!item->isValidInstance()) {
                    return QColor(255, 95, 82);
                }
                return QColor(88, 165, 240);
            }
        } break;
        case Qt::SizeHintRole: {
            if(index.column() == 2) {
                return QSize(16, 1);
            }
        } break;
        case Qt::UserRole: {
            return QString::number(item->uuid());
        }
        default: break;
    }
    return QVariant();
}

bool ObjectHierarchyModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    Q_UNUSED(role)
    Object *item = static_cast<Object* >(index.internalPointer());
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
            case 2: return "";
            default: break;
        }
    }
    return QVariant();
}

int ObjectHierarchyModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 3;
}

int ObjectHierarchyModel::rowCount(const QModelIndex &parent) const {
    if(m_rootItem) {
        Object *parentItem = m_rootItem;
        if(parent.isValid()) {
            parentItem = static_cast<Object *>(parent.internalPointer());
        }
        const Object::ObjectList &children = parentItem->getChildren();
        return children.size();
    }
    return 0;
}

QModelIndex ObjectHierarchyModel::index(int row, int column, const QModelIndex &parent) const {
    if(m_rootItem) {
        Object *parentItem = m_rootItem;
        if(parent.isValid()) {
            parentItem = static_cast<Object *>(parent.internalPointer());
        }

        const Object::ObjectList &children = parentItem->getChildren();
        if(row >= children.size() || row < 0) {
            return QModelIndex();
        }
        QList<Object *> list;
        for(auto it : children) {
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

    Object *childItem = static_cast<Object *>(index.internalPointer());
    Object *parentItem = static_cast<Object *>(childItem->parent());

    if(!parentItem || parentItem == m_rootItem) {
        return QModelIndex();
    }
    QList<Object *> list;
    Object *p = parentItem->parent();
    if(p) {
        for(auto it : p->getChildren()) {
            list.push_back(it);
        }
    }
    return createIndex(list.indexOf(parentItem), 0, parentItem);
}

Qt::ItemFlags ObjectHierarchyModel::flags(const QModelIndex &index) const {
    if(!index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    Qt::ItemFlags result = Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(index.column() == 0) {
        result  |= Qt::ItemIsEditable;
    }
    return result;
}
