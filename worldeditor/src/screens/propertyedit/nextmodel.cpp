#include "nextmodel.h"

#include "property.h"

NextModel::NextModel(QObject *parent):
        PropertyModel(parent) {
}

void NextModel::addItem(Object *propertyObject) {
    const MetaObject *metaObject = propertyObject->metaObject();

    int i = rowCount();
    Property *propertyItem = static_cast<Property *>(m_rootItem);

    int count = metaObject->propertyCount();
    if(count) {
        beginInsertRows(QModelIndex(), i, i + 1);

        QString name = metaObject->name();

        propertyItem = new Property(name, static_cast<Property *>(m_rootItem), true, true);
        propertyItem->setPropertyObject(propertyObject);

        for(int i = 0; i < count; i++) {
            MetaProperty property = metaObject->property(i);

            Property *p = new Property(property.name(), (propertyItem) ? propertyItem : static_cast<Property *>(m_rootItem), false, false);
            p->setPropertyObject(propertyObject);
        }

        endInsertRows();
    }

    if(propertyItem) {
        updateDynamicProperties(propertyItem, propertyObject);
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void NextModel::updateDynamicProperties(Property *parent, Object *propertyObject) {
    // Get dynamic property names
    std::list<std::string> stringList = propertyObject->dynamicPropertyNames();
    QStringList dynamicProperties;
    for(auto it : propertyObject->dynamicPropertyNames()) {
        dynamicProperties << it.c_str();
    }

    Property *p = dynamic_cast<Property *>(m_rootItem);
    for(int i = 0; i < dynamicProperties.size(); i++) {
        QString name(dynamicProperties[i]);
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

    for(QString &dynProp : dynamicProperties) {
        QStringList list = dynProp.split('/');

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
                    p = new Property(path, it, true, true);
                    p->setPropertyObject(propertyObject);

                    it = p;
                }
            } else if(!list[i].isEmpty()) {
                p = new Property(dynProp, it, false, false);
                p->setPropertyObject(propertyObject);

                p->setProperty("__Dynamic", true);
            }
        }
        it = s;
    }
}
