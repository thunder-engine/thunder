#include "propertymodel.h"

#include "custom/Property.h"
#include "custom/EnumProperty.h"

#include <QApplication>
#include <QMetaProperty>
#include <QItemEditorFactory>

struct PropertyPair {
    PropertyPair(const QMetaObject *obj, QMetaProperty property):
        Property(property),
        Object(obj) {
    }

    QMetaProperty           Property;
    const QMetaObject      *Object;

    bool operator==(const PropertyPair& other) const {
        return QString(other.Property.name()) == QString(Property.name());
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
    Property *item  = static_cast<Property* >(index.internalPointer());
    switch(role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole:
        case Qt::EditRole: {
            if(index.column() == 0) {
                return item->name().replace('_', ' ');
            }
            if(index.column() == 1) {
                return item->value(role);
            }
        } break;
        case Qt::BackgroundRole: {
            if(item->isRoot()) {
                return QApplication::palette("QTreeView").brush(QPalette::Normal, QPalette::Button).color();
            }
        } break;
        case Qt::DecorationRole: {
            if(index.column() == 1) {
                return item->value(role);
            }
        } break;
        case Qt::FontRole: {
            if(item->isRoot()) {
                QFont font  = QApplication::font("QTreeView");
                font.setBold(true);
                font.setPointSize(10);
                return font;
            }
        } break;
        case Qt::SizeHintRole: {
            if(item->isRoot()) {
                return QSize(1, 32);
            }
            return QSize(1, 24);
        } break;

        default: break;
    }

    return QVariant();
}

// edit methods
bool PropertyModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(index.isValid() && role == Qt::EditRole) {
        Property *item      = static_cast<Property *>(index.internalPointer());
        item->setValue(value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags PropertyModel::flags(const QModelIndex &index) const {
    if(!index.isValid())
        return Qt::ItemIsEnabled;
    Property *item = static_cast<Property*>(index.internalPointer());
    // only allow change of value attribute
    if(item->isRoot()) {
        return Qt::ItemIsEnabled;
    } else if (item->isReadOnly()) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
    } else {
        return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
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

QModelIndex PropertyModel::buddy(const QModelIndex &index) const  {
    if(index.isValid() && index.column() == 0) {
        return createIndex(index.row(), 1, index.internalPointer());
    }
    return index;
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
                if( property.isUser(propertyObject) ) {   // Hide Qt specific properties
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
        bool keep       = false; // false
        foreach(PropertyPair pair, propertyMap) {
            if(pair.Object == obj) {
                keep    = true;
                break;
            }
        }
        if(keep) {
            finalClassList.push_back(obj);
        }
    }
    // finally insert properties for classes containing them
    int i = rowCount();
    Property *propertyItem  = static_cast<Property *>(parent);
    beginInsertRows( QModelIndex(), i, i + finalClassList.count() );
    foreach(const QMetaObject *metaObject, finalClassList) {
        QString name    = propertyObject->objectName();
        if(name.isEmpty()) {
            // Set default name of the hierarchy property to the class name
            name        = metaObject->className();
            // Check if there is a special name for the class
            int index   = metaObject->indexOfClassInfo(qPrintable(name));
            if (index != -1) {
                name    = metaObject->classInfo(index).value();
            }
        }
        // Create Property Item for class node
        if(!parent) {
            propertyItem    = new Property(name, propertyObject, m_rootItem, true);
        }

        foreach(PropertyPair pair, propertyMap) {
            // Check if the property is associated with the current class from the finalClassList
            if(pair.Object == metaObject) {
                QMetaProperty property(pair.Property);
                Property *p = nullptr;
                if (!m_userCallbacks.isEmpty()) {
                    QList<PropertyEditor::UserTypeCB>::iterator iter = m_userCallbacks.begin();
                    while( p == nullptr && iter != m_userCallbacks.end() ) {
                        p   = (*iter)(property.name(), propertyObject, propertyItem);
                        ++iter;
                    }
                }
                if(p == nullptr) {
                    if(property.isEnumType()) {
                        p   = new EnumProperty(property.name(), propertyObject, propertyItem);
                    } else {
                        p   = new Property(property.name(), propertyObject, (propertyItem) ? propertyItem : m_rootItem);
                    }
                }
                p->setName(propertyName);

                int index   = metaObject->indexOfClassInfo(property.name());
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

void PropertyModel::updateItem(QObject *propertyObject) {
    if(!propertyObject) {
        propertyObject  = m_rootItem;
    }

    QList<QObject*> childs  = propertyObject->children();
    for(int i = 0; i < childs.count(); i++) {
        Property *p = dynamic_cast<Property *>(childs[i]);
        if(p) {
            if(!p->isRoot()) {
                if(!p->value().isValid()) {
                    p->setParent(nullptr);
                    p->deleteLater();
                }
            } else {
                updateItem(p);
                if(p->children().isEmpty()) {
                    p->setParent(nullptr);
                    p->deleteLater();
                } else {
                    updateDynamicProperties(p, p->propertyObject());
                }
            }
        }
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void PropertyModel::updateDynamicProperties(Property *parent, QObject *propertyObject ) {
    // Get dynamic property names
    QList<QByteArray> dynamicProperties = propertyObject->dynamicPropertyNames();

    Property *p = dynamic_cast<Property *>(m_rootItem);
    for(int i = 0; i < dynamicProperties.size(); i++) {
        QByteArray name = dynamicProperties[i];
        QObject *object = p->findChild<QObject *>(name);
        if(object) {
            dynamicProperties.removeAll(name);
            i   = -1;
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

    Property *it    = parent;
    // Add properties left in the list
    foreach(QByteArray dynProp, dynamicProperties) {
        QList<QByteArray> list  = dynProp.split('/');

        Property *s = it;
        it          = (list.size() > 1) ? dynamic_cast<Property *>(m_rootItem) : it;
        for(int i = 0; i < list.size(); i++) {
            Property *p = nullptr;

            if(it && i < list.size() - 1) {
                Property *child = it->findChild<Property *>(list[i]);
                if(child) {
                    it  = child;
                } else {
                    p   = new Property(list[i], propertyObject, it, true);
                    it  = p;
                }
            } else if(!list[i].isEmpty()) {
                if(!m_userCallbacks.isEmpty()) {
                    QList<PropertyEditor::UserTypeCB>::iterator iter = m_userCallbacks.begin();
                    while( p == nullptr && iter != m_userCallbacks.end() ) {
                        p   = (*iter)(dynProp, propertyObject, it);
                        ++iter;
                    }
                }
                if(p == nullptr) {
                    p   = new Property(dynProp, propertyObject, it);
                }
                p->setProperty("__Dynamic", true);
            }
        }
        it  = s;
    }
}

void PropertyModel::clear() {
    delete m_rootItem;
    m_rootItem  = new Property("Root", nullptr, this);

    beginResetModel();
    endResetModel();
}

void PropertyModel::registerCustomPropertyCB(PropertyEditor::UserTypeCB callback) {
    if(!m_userCallbacks.contains(callback)) {
        m_userCallbacks.push_back(callback);
    }
}

void PropertyModel::unregisterCustomPropertyCB(PropertyEditor::UserTypeCB callback) {
    int index = m_userCallbacks.indexOf(callback);
    if(index != -1) {
        m_userCallbacks.removeAt(index);
    }
}
