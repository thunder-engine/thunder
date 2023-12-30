#include "propertymodel.h"

#include <editor/property.h>

#include <QApplication>
#include <QMetaProperty>
#include <QRegularExpression>

QString fromCamelCase(const QString &s) {
    static QRegularExpression regExp1 {"(.)([A-Z][a-z]+)"};
    static QRegularExpression regExp2 {"([a-z0-9])([A-Z])"};

    QString result = s;
    result.replace(regExp1, "\\1 \\2");
    result.replace(regExp2, "\\1 \\2");

    result[0] = result[0].toUpper();

    return result;
}

struct PropertyPair {
    PropertyPair(const QMetaObject *obj, QMetaProperty property):
        property(property),
        object(obj) {
    }

    QMetaProperty property;
    const QMetaObject *object;

    bool operator==(const PropertyPair& other) const {
        return QString(other.property.name()) == QString(property.name());
    }
};

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
        //case Qt::TextAlignmentRole: {
        //    if(index.column() == 0) {
        //        return Qt::AlignTop;
        //    }
        //}
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

void PropertyModel::addItem(QObject *propertyObject, const QString &propertyName, QObject *parent) {
    // first create property <-> class hierarchy
    QList<PropertyPair> propertyMap;
    QList<const QMetaObject *> classList;
    const QMetaObject *metaObject = propertyObject->metaObject();
    do {
        int count = metaObject->propertyCount();
        if(count) {
            for(int i = 0; i < count; i++) {
                QMetaProperty property = metaObject->property(i);
                if(property.isUser(propertyObject)) { // Hide Qt specific properties
                    PropertyPair pair(metaObject, property);
                    int index = propertyMap.indexOf(pair);
                    if(index != -1) {
                        propertyMap[index] = pair;
                    } else {
                        propertyMap.push_back(pair);
                    }
                }
            }
            classList.push_front(metaObject);
        }
    } while((metaObject = metaObject->superClass()) != nullptr);
	
    QList<const QMetaObject*> finalClassList;
    // remove empty classes from hierarchy list
    foreach(const QMetaObject* obj, classList) {
        bool keep = false; // false
        foreach(PropertyPair pair, propertyMap) {
            if(pair.object == obj) {
                keep = true;
                break;
            }
        }
        if(keep) {
            finalClassList.push_back(obj);
        }
    }

    // finally insert properties for classes containing them
    int i = rowCount();
    Property *propertyItem = static_cast<Property *>((parent == nullptr) ? m_rootItem : parent);
    beginInsertRows(QModelIndex(), i, i + finalClassList.count());
    for(const QMetaObject *metaObject : finalClassList) {
        QString name = propertyObject->objectName();
        if(name.isEmpty()) {
            // Set default name of the hierarchy property to the class name
            name = metaObject->className();
            // Check if there is a special name for the class
            int index = metaObject->indexOfClassInfo(qPrintable(name));
            if (index != -1) {
                name = metaObject->classInfo(index).value();
            }
        }
        // Create Property Item for class node
        if(!parent) {
            propertyItem = new Property(name, propertyObject, m_rootItem, true);
        }

        for(PropertyPair &pair : propertyMap) {
            // Check if the property is associated with the current class from the finalClassList
            if(pair.object == metaObject) {
                QMetaProperty property(pair.property);
                Property *p = Property::constructProperty(property.name(), propertyObject, propertyItem, false);
                if(p == nullptr) {
                    p = new Property(property.name(), propertyObject, (propertyItem) ? propertyItem : m_rootItem);
                }

                p->setName(propertyName);

                int index = metaObject->indexOfClassInfo(property.name());
                if(index != -1) {
                    p->setEditorHints(metaObject->classInfo(index).value());
                }
            }
        }
    }
    endInsertRows();
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
        it = (list.size() > 1) ? dynamic_cast<Property *>(m_rootItem) : it;
        for(int i = 0; i < list.size(); i++) {
            Property *p = nullptr;

            if(it && i < list.size() - 1) {
                QString path = list.mid(0, i + 1).join('/');
                Property *child = it->findChild<Property *>(path);
                if(child) {
                    it = child;
                } else {
                    p = Property::constructProperty(path, propertyObject, it, true);
                    if(p == nullptr) {
                        p = new Property(path, propertyObject, it, true);
                    }
                    it = p;
                }
            } else if(!list[i].isEmpty()) {
                p = Property::constructProperty(dynProp, propertyObject, it, false);
                if(p == nullptr) {
                    p = new Property(dynProp, propertyObject, it);
                }

                p->setProperty("__Dynamic", true);
            }
        }
        it = s;
    }
}

void PropertyModel::clear() {
    delete m_rootItem;
    m_rootItem = new Property("Root", nullptr, this);

    beginResetModel();
    endResetModel();
}
