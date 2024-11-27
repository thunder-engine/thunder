#include "propertymodel.h"

#include <QApplication>
#include <QMetaProperty>
#include <QRegularExpression>

#include "property.h"
#include "propertyeditor.h"

QString fromCamelCase(const QString &s) {
    static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
    static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

    QString result = s;
    result.replace(regExp1, "\\1 \\2");
    result.replace(regExp2, "\\1 \\2");

    result[0] = result[0].toUpper();

    return result;
}

PropertyModel::PropertyModel(QObject *parent):
        BaseObjectModel(parent) {
}

PropertyModel::~PropertyModel() {
    delete m_rootItem;
}

int PropertyModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 2;
}

QVariant PropertyModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) {
        return QVariant();
    }
    Property *item = static_cast<Property* >(index.internalPointer());
    switch(role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole:
        case Qt::EditRole: {
            if(index.column() == 0) {
                return fromCamelCase(item->name().replace('_', ' '));
            }
            if(index.column() == 1 && (item->editor() == nullptr || role == Qt::EditRole)) {
                return item->value(role);
            }
        } break;
        case Qt::BackgroundRole: {
            if(item->isRoot()) {
                return QApplication::palette("QTreeView").brush(QPalette::Normal, QPalette::Button).color();
            }
            if(index.column() == 0) {
                return QColor("#ff0000ff");
            }
        } break;
        case Qt::DecorationRole: {
            if(index.column() == 1) {
                return item->value(role);
            }
        } break;
        case Qt::FontRole: {
            if(item->isRoot()) {
                QFont font = QApplication::font("QTreeView");
                font.setBold(true);
                font.setPointSize(10);
                return font;
            }
        } break;
        case Qt::SizeHintRole: {
            return QSize(1, 26);
        }
        case Qt::CheckStateRole: {
            if(index.column() == 0 && item->isCheckable()) {
                return item->isChecked() ? Qt::Checked : Qt::Unchecked;
            }
        } break;
        default: break;
    }

    return QVariant();
}

// edit methods
bool PropertyModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(!index.isValid()) {
        return false;
    }
    Property *item = static_cast<Property *>(index.internalPointer());
    if(role == Qt::EditRole) {
        item->setValue(value);
        emit dataChanged(index, index);
        return true;
    } else if(role == Qt::CheckStateRole) {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        item->setChecked(state == Qt::Checked);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags PropertyModel::flags(const QModelIndex &index) const {
    if(!index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    Property *item = static_cast<Property *>(index.internalPointer());
    // only allow change of value attribute

    Qt::ItemFlags result;
    if(item->isRoot()) {
        result |= Qt::ItemIsEnabled;
    } else if(item->isReadOnly()) {
        result |= Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
    } else {
        result |= Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;

        if(index.column() == 1) {
            result |= Qt::ItemIsEditable;
        }
    }

    if(index.column() == 0 && item->isCheckable()) {
        result |= Qt::ItemIsUserCheckable;
    }

    return result;
}

QVariant PropertyModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Property");
            case 1: return tr("Value");
        }
    }
    return QVariant();
}

void PropertyModel::addItem(QObject *propertyObject) {
    const QMetaObject *metaObject = propertyObject->metaObject();

    Property *propertyItem = static_cast<Property *>(m_rootItem);

    int count = metaObject->propertyCount();
    if(count) {
        QString name = propertyObject->objectName();
        if(name.isEmpty()) {
            name = metaObject->className();
        }
        // Create Property Item for class node
        propertyItem = new Property(name, static_cast<Property *>(m_rootItem), true);
        propertyItem->setPropertyObject(propertyObject);

        bool empty = true;
        for(int i = 0; i < count; i++) {
            QMetaProperty property = metaObject->property(i);

            if(property.isUser(propertyObject)) { // Hide Qt specific properties
                empty = false;

                Property *p = new Property(property.name(), (propertyItem) ? propertyItem : static_cast<Property *>(m_rootItem), false);
                p->setPropertyObject(propertyObject);

                int index = metaObject->indexOfClassInfo(property.name());
                if(index != -1) {
                    p->setEditorHints(metaObject->classInfo(index).value());
                }
            }
        }

        if(empty) {
            delete propertyItem;
            propertyItem = static_cast<Property *>(m_rootItem);
        } else {
            int i = rowCount();
            beginInsertRows(QModelIndex(), i, i + 1);

            endInsertRows();
        }
    }

    if(propertyItem) {
        updateDynamicProperties(propertyItem, propertyObject);
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void PropertyModel::updateDynamicProperties(Property *parent, QObject *propertyObject) {
    // Get dynamic property names
    QList<QByteArray> dynamicProperties = propertyObject->dynamicPropertyNames();

    Property *p = dynamic_cast<Property *>(m_rootItem);
    for(int i = 0; i < dynamicProperties.size(); i++) {
        QByteArray name = dynamicProperties[i];
        QObject *object = p->findChild<QObject *>(name);
        if(object) {
            dynamicProperties.removeAll(name);
            i = -1;
        }
    }

    // Remove invalid properites and those we don't want to add
    for(int i = 0; i < dynamicProperties.size(); i++) {
         QString dynProp = dynamicProperties[i];
         // Skip properties starting with _ (because there may be dynamic properties from Qt with _q_ and we may
         // have user defined hidden properties starting with _ too
         if(dynProp.startsWith("_") || !propertyObject->property(qPrintable(dynProp)).isValid()) {
             dynamicProperties.removeAt(i);
             --i;
         }
    }

    if(dynamicProperties.empty()) {
        return;
    }

    Property *it = parent;

    // Add properties left in the list
    for(QByteArray &dynProp : dynamicProperties) {
        QByteArrayList list = dynProp.split('/');

        Property *s = it;
        it = (list.size() > 1) ? static_cast<Property *>(m_rootItem) : it;
        for(int i = 0; i < list.size(); i++) {
            Property *p = nullptr;

            if(it && i < list.size() - 1) {
                QString path = list.mid(0, i + 1).join('/');
                Property *child = it->findChild<Property *>(path);
                if(child) {
                    it = child;
                } else {
                    p = new Property(path, it, true);
                    p->setPropertyObject(propertyObject);
                    it = p;
                }
            } else if(!list[i].isEmpty()) {
                p = new Property(dynProp, it, false);
                p->setPropertyObject(propertyObject);
                p->setProperty("__Dynamic", true);
            }
        }
        it = s;
    }
}

void PropertyModel::clear() {
    delete m_rootItem;
    m_rootItem = new Property("Root", nullptr, true);

    beginResetModel();
    endResetModel();
}
