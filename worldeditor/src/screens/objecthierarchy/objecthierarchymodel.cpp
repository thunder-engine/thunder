#include "objecthierarchymodel.h"

#include <QApplication>
#include <QFont>
#include <QBrush>

#include "components/actor.h"
#include "components/scene.h"
#include "components/world.h"
#include "resources/prefab.h"

ObjectHierarchyModel::ObjectHierarchyModel(QObject *parent) :
        QAbstractItemModel(parent),
        m_rootItem(nullptr),
        m_showNone(false),
        m_visible(":/Style/styles/dark/icons/eye.png"),
        m_invisible(":/Style/styles/dark/icons/eye_close.png"),
        m_select(":/Style/styles/dark/icons/select.png"),
        m_selectDisable(":/Style/styles/dark/icons/select_disable.png"),
        m_prefab(":/Style/styles/dark/icons/prefab.png"),
        m_actor(":/Style/styles/dark/icons/actor.png") {

}

void ObjectHierarchyModel::setRoot(Object *root) {
    m_rootItem = root;

    beginResetModel();
    endResetModel();
}

Object *ObjectHierarchyModel::root() const {
    return m_rootItem;
}

void ObjectHierarchyModel::showNone() {
    m_showNone = true;
}

QVariant ObjectHierarchyModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }
    uint32_t internalId = index.internalId();
    Object *object = Engine::findObject(internalId);
    Actor *actor = dynamic_cast<Actor *>(object);
    Scene *scene = dynamic_cast<Scene *>(object);

    switch(role) {
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::DisplayRole: {
            if(object) {
                switch(index.column()) {
                    case 0: return QString::fromStdString(object->name()) + ((scene && scene->isModified()) ? " *" : "");
                    case 1: return QString::fromStdString(object->typeName());
                    default: break;
                }
            } else {
                if(index.column() == 0) {
                    return "None";
                } else {
                    return QString();
                }
            }
        } break;
        case Qt::DecorationRole: {
            if(actor) {
                switch(index.column()) {
                    case 0: return actor->isInstance() ? m_prefab : m_actor;
                    case 2: return (actor->hideFlags() & Actor::ENABLE) ? m_visible : m_invisible;
                    case 3: return (actor->hideFlags() & Actor::SELECTABLE) ? QVariant() : m_selectDisable;
                    default: break;
                }
            }
        } break;
        case Qt::BackgroundRole: {
            if(index.column() == 2 || index.column() == 3) {
                return QBrush(QColor(0, 0, 0, 128));
            }
        } break;
        case Qt::FontRole: {
            if(scene && static_cast<World *>(m_rootItem)->activeScene() == scene) {
                QFont font = QApplication::font("QTreeView");
                font.setBold(true);
                font.setPointSize(10);
                return font;
            }
        } break;
        case Qt::ForegroundRole: {
            if(actor && actor->isInstance()) {
                if(Engine::reference(actor->prefab()).empty()) {
                    return QBrush(QColor(255, 95, 82));
                }
                return QBrush(QColor(88, 165, 240));
            }
        } break;
        case Qt::UserRole: {
            if(object) {
                return QString::number(object->uuid());
            }
            return QString();
        }
        default: break;
    }
    return QVariant();
}

bool ObjectHierarchyModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    Q_UNUSED(role)
    Object *item = Engine::findObject(index.internalId());
    if(item) {
        item->setName(value.toString().toStdString());
    }
    return true;
}

QVariant ObjectHierarchyModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Name");
            case 1: return tr("Type");
            case 2: return "";
            case 3: return "";
            default: break;
        }
    }
    return QVariant();
}

int ObjectHierarchyModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 4;
}

int ObjectHierarchyModel::rowCount(const QModelIndex &parent) const {
    if(m_rootItem) {
        Object *parentItem = m_rootItem;
        if(parent.isValid()) {
            parentItem = Engine::findObject(parent.internalId());
        }

        if(parentItem) {
            const Object::ObjectList &children = parentItem->getChildren();
            int result = children.size();
            if(m_showNone && parentItem == m_rootItem) {
                result++;
            }
            return result;
        }
    }
    return 0;
}

QModelIndex ObjectHierarchyModel::index(int row, int column, const QModelIndex &parent) const {
    if(m_rootItem) {
        Object *ptr = nullptr;
        Object *parentItem = m_rootItem;
        if(parent.isValid()) {
            parentItem = Engine::findObject(parent.internalId());
        }

        if(parentItem) {
            const Object::ObjectList &children = parentItem->getChildren();
            int count = children.size();
            if(m_showNone && parentItem == m_rootItem) {
                count++;
            }
            if(row >= count || row < 0) {
                return QModelIndex();
            }

            if(!m_showNone || parentItem != m_rootItem) {
                ptr = *std::next(children.begin(), row);
            } else if(row != 0) {
                ptr = *std::next(children.begin(), row-1);
            }
        }
        if(ptr) {
            return createIndex(row, column, ptr->uuid());
        }
    }
    return QModelIndex();
}

QModelIndex ObjectHierarchyModel::parent(const QModelIndex &index) const {
    if(!index.isValid()) {
        return QModelIndex();
    }

    Object *childItem = Engine::findObject(index.internalId());
    if(childItem == nullptr) {
        return QModelIndex();
    }
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
    return createIndex(list.indexOf(parentItem), 0, parentItem->uuid());
}

Qt::ItemFlags ObjectHierarchyModel::flags(const QModelIndex &index) const {
    if(!index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    Qt::ItemFlags result = Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if(index.column() == 0) {
        result |= Qt::ItemIsEditable;
    }
    return result;
}
