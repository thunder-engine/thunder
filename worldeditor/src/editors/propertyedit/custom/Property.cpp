#include <QMetaProperty>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPainter>

#include "Property.h"

#include "../editors/Actions.h"

#include "../nextobject.h"

Property::Property(const QString &name, QObject *propertyObject, QObject *parent, bool root) :
        QObject(parent),
        m_propertyObject(propertyObject),
        m_editor(nullptr),
        m_root(root),
        m_checkable(false) {

    QStringList list = name.split('/');

    m_name = list.back();
    setObjectName(name);
    m_root = (root) ? (list.size() == 1) : false;
    m_checkable = m_root;
}

QVariant Property::value(int role) const {
    if(m_propertyObject && role != Qt::DecorationRole) {
        return m_propertyObject->property(qPrintable(objectName()));
    }
    return QVariant();
}

void Property::setValue(const QVariant &value) {
    if(m_propertyObject) {
        m_propertyObject->setProperty(qPrintable(objectName()), value);
    }
}

QString Property::name() const {
    if(m_name.length() != 0)
        return m_name;

    return objectName();
}

bool Property::isReadOnly() const {
    if(m_propertyObject && m_propertyObject->dynamicPropertyNames().contains( objectName().toLocal8Bit() )) {
        return false;
    } else if(m_propertyObject && m_propertyObject->metaObject()->property(m_propertyObject->metaObject()->indexOfProperty(qPrintable(objectName()))).isWritable()) {
        return false;
    }
    return true;
}

QWidget *Property::createEditor(QWidget *parent, const QStyleOptionViewItem &) {
    NextObject *next = dynamic_cast<NextObject *>(m_propertyObject);
    if(m_root && next) {
        Actions *act = new Actions(m_name, parent);
        Object *object = next->component(objectName());
        act->setObject(object);
        act->setMenu(next->menu(object));

        m_editor = act;
    }
    return m_editor;
}

QSize Property::sizeHint(const QSize &size) const {
    return size;
}

bool Property::setEditorData(QWidget *, const QVariant &) {
    return false;
}

QVariant Property::editorData(QWidget *) {
    return QVariant();
}

Property *Property::findPropertyObject(QObject *propertyObject) {
    if(m_propertyObject == propertyObject) {
        return this;
    }
    for(int i = 0; i < children().size(); ++i) {
        Property *child = static_cast<Property *>(children()[i])->findPropertyObject(propertyObject);
        if(child) {
            return child;
        }
    }
    return nullptr;
}

void Property::setChecked(bool value) {
    if(!m_override.isEmpty() && m_propertyObject) {
        m_propertyObject->setProperty(qPrintable(m_override), value);
    }
    if(m_editor) {
        static_cast<Actions *>(m_editor)->onDataChanged(value);
    }
}

bool Property::isChecked() const {
    if(!m_override.isEmpty() && m_propertyObject) {
        return m_propertyObject->property(qPrintable(m_override)).toBool();
    }
    if(m_editor) {
        return static_cast<Actions *>(m_editor)->isChecked();
    }
    return false;
}

void Property::setOverride(const QString &property) {
    m_override = property;
    m_checkable = true;
}
