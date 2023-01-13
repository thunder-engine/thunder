#include "Property.h"

#include <QMetaProperty>
#include <QWidget>

QList<Property::UserTypeCallback> Property::m_userCallbacks;

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
}

QString Property::editorHints() const {
    return m_hints;
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

void Property::setEditorHints(const QString &hints) {
    m_hints = hints;
}

QString Property::name() const {
    if(m_name.length() != 0)
        return m_name;

    return objectName();
}

void Property::setName(const QString &value) {
    m_name = value;
}

QWidget *Property::editor() const {
    return m_editor;
}

QObject *Property::propertyObject() const {
    return m_propertyObject;
}

bool Property::isRoot() const {
    return m_root;
}

bool Property::isReadOnly() const {
    if(m_propertyObject) {
        const QMetaObject *meta = m_propertyObject->metaObject();
        if(m_propertyObject->dynamicPropertyNames().contains( objectName().toLocal8Bit() )) {
            return false;
        } else if(meta->property(meta->indexOfProperty(qPrintable(objectName()))).isWritable()) {
            return false;
        }
    }
    return true;
}

bool Property::isCheckable() const {
    return m_checkable;
}

bool Property::isPersistent() const {
    return true;
}

QWidget *Property::getEditor(QWidget *parent) const {
    if(m_editor) {
        if(parent) {
            m_editor->setParent(parent);
        }
        return m_editor;
    }
    return createEditor(parent);
}

QWidget *Property::createEditor(QWidget *parent) const {
    return m_editor;
}

QSize Property::sizeHint(const QSize &size) const {
    QWidget *widget = getEditor(nullptr);
    return QSize(size.width(), widget ? widget->height() : size.height());
}

bool Property::setEditorData(QWidget *, const QVariant &) {
    return false;
}

QVariant Property::editorData(QWidget *) {
    return QVariant();
}

bool Property::isChecked() const {
    return false;
}

void Property::setChecked(bool value) {

}

void Property::registerPropertyFactory(UserTypeCallback callback) {
    if(!m_userCallbacks.contains(callback)) {
        m_userCallbacks.push_back(callback);
    }
}

void Property::unregisterPropertyFactory(UserTypeCallback callback) {
    int index = m_userCallbacks.indexOf(callback);
    if(index != -1) {
        m_userCallbacks.removeAt(index);
    }
}

Property *Property::constructProperty(const QString &name, QObject *propertyObject, Property *parent, bool root) {
    Property *result = nullptr;
    if(!m_userCallbacks.isEmpty()) {
        auto iter = m_userCallbacks.begin();
        while(result == nullptr && iter != m_userCallbacks.end() ) {
            result = (*iter)(name, propertyObject, parent, root);
            ++iter;
        }
    }
    return result;
}
