#include "nextmodel.h"

#include "property.h"

NextModel::NextModel(QObject *parent):
        PropertyModel(parent) {
}

void NextModel::addItem(Object *propertyObject) {
    const MetaObject *metaObject = propertyObject->metaObject();

    Property *propertyItem = static_cast<Property *>(m_rootItem);

    int count = metaObject->propertyCount();
    if(count) {
        QString name = metaObject->name();

        propertyItem = new Property(name, static_cast<Property *>(m_rootItem), true);
        propertyItem->setPropertyObject(propertyObject);

        connect(propertyItem, &Property::propertyChanged, this, &NextModel::propertyChanged);

        for(int i = 0; i < count; i++) {
            MetaProperty property = metaObject->property(i);

            if(!QString(property.name()).toLower().contains("enable")) {
                uint32_t type = property.read(propertyObject).type();
                if(type < MetaType::QUATERNION || type >= MetaType::OBJECT) {
                    Property *p = new Property(property.name(), (propertyItem) ? propertyItem : static_cast<Property *>(m_rootItem), false);
                    p->setPropertyObject(propertyObject);

                    p->setEditorHints(property.table()->annotation);

                    connect(p, &Property::propertyChanged, this, &NextModel::propertyChanged);
                }
            }
        }
    }

    if(propertyItem) {
        updateDynamicProperties(propertyItem, propertyObject);
    }

    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void NextModel::updateDynamicProperties(Property *parent, Object *propertyObject) {
    // Get dynamic property names
    auto dynamicProperties = propertyObject->dynamicPropertyNames();

    QStringList dynamicPropertiesFiltered;
    // Remove invalid properites and those we don't want to add
    for(auto it : dynamicProperties) {
        // Skip user defined hidden properties starting with _
        if(it.front() != '_') {
            dynamicPropertiesFiltered << it.c_str();
        }
    }

    if(dynamicPropertiesFiltered.empty()) {
        return;
    }

    Property *it = parent;
    // Add properties left in the list

    for(QString &dynProp : dynamicPropertiesFiltered) {
        QStringList list = dynProp.split('/');

        Property *s = it;
        for(int i = 0; i < list.size(); i++) {
            Property *p = nullptr;

            if(it && i < list.size() - 1) {
                QString path = list.mid(0, i + 1).join('/');
                Property *child = it->findChild<Property *>(path);
                if(child) {
                    it = child;
                } else {
                    p = new Property(path, it, parent == m_rootItem);
                    p->setPropertyObject(propertyObject);

                    it = p;
                }
            } else if(!list[i].isEmpty()) {
                p = new Property(dynProp, it, false);
                p->setPropertyObject(propertyObject);

                std::string annotationName("_" + dynProp.toStdString() + "Annotation");
                for(auto it : dynamicProperties) {
                    if(it == annotationName) {
                        std::string value = propertyObject->property(annotationName.c_str()).toString();
                        p->setEditorHints(value.c_str());
                        break;
                    }
                }

                connect(p, &Property::propertyChanged, this, &NextModel::propertyChanged);

                p->setProperty("__Dynamic", true);
            }
        }
        it = s;
    }
}
