#include "property.h"

#include <invalid.h>
#include <editor/propertyedit.h>

#include <QMetaProperty>

#include "nextobject.h"
#include "custom/component/actions.h"

Property::Property(const QString &name, Property *parent, bool root, bool second) :
        QObject(parent),
        m_propertyObject(nullptr),
        m_nextObject(nullptr),
        m_editor(nullptr),
        m_root(root),
        m_second(second),
        m_checkable(false) {

    QStringList list = name.split('/');

    m_name = list.back();
    setObjectName(name);
    m_root = (root) ? (list.size() == 1) : false;
}

void Property::setPropertyObject(QObject *propertyObject) {
    m_propertyObject = propertyObject;

    NextObject *next = dynamic_cast<NextObject *>(m_propertyObject);
    if(next) {
        m_hints = next->propertyHint(m_name);

        m_checkable = m_root && m_second;

        m_nextObject = next->component(objectName());

        Invalid *invalid = dynamic_cast<Invalid *>(m_nextObject);
        if(invalid) {
            m_name += " (Invalid)";
        }
    }
}

void Property::setPropertyObject(Object *propertyObject) {
    m_nextObject = propertyObject;
}

QString Property::editorHints() const {
    return m_hints;
}

QVariant Property::value(int role) const {
    if(role == Qt::DisplayRole) {
        PropertyEdit *editor = dynamic_cast<PropertyEdit *>(m_editor);
        if(editor) {
            return editor->data();
        }
    } else if(role != Qt::DecorationRole) {
        if(m_propertyObject) {
            return m_propertyObject->property(qPrintable(objectName()));
        }
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
    if(m_name.length() != 0) {
        return m_name;
    }

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
        NextObject *next = dynamic_cast<NextObject *>(m_propertyObject);
        if(next) {
            return next->isReadOnly(objectName());
        }

        const QMetaObject *meta = m_propertyObject->metaObject();
        if(m_propertyObject->dynamicPropertyNames().contains( objectName().toLocal8Bit() )) {
            return false;
        } else if(meta->property(meta->indexOfProperty(qPrintable(objectName()))).isWritable()) {
            return false;
        }
        return true;
    }

    return false;
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
    int32_t type = 0;

    if(m_propertyObject) {
        const QMetaObject *meta = m_propertyObject->metaObject();
        int32_t index = meta->indexOfProperty(qPrintable(objectName()));
        if(index > -1) {
            QMetaProperty property = meta->property(index);
            if(property.isEnumType()) {
                type = -1;
            } else {
                QVariant data = property.read(m_propertyObject);
                type = data.userType();
            }
        } else {
            index = m_propertyObject->dynamicPropertyNames().indexOf(qPrintable(objectName()));
            if(index > -1) {
                QVariant data = m_propertyObject->property(qPrintable(objectName()));
                type = data.userType();
            }
        }
    }

    PropertyEdit *editor = PropertyEdit::constructEditor(type, parent, objectName(), m_propertyObject);
    if(editor == nullptr && m_root) {
        editor = new Actions(parent);
    }

    if(editor) {
        editor->setObject(m_propertyObject, objectName());

        editor->setDisabled(isReadOnly());
        editor->setEditorHint(m_hints);

        connect(editor, &PropertyEdit::editFinished, this, &Property::onDataChanged);
        connect(editor, &PropertyEdit::dataChanged, this, &Property::onDataChanged);
        connect(editor, &Actions::destroyed, this, &Property::onEditorDestoyed);
        m_editor = editor;
    }

    return m_editor;
}

QSize Property::sizeHint(const QSize &size) const {
    return QSize(size.width(), m_editor ? m_editor->height() : size.height());
}

QVariant Property::editorData(QWidget *editor) {
    PropertyEdit *e = dynamic_cast<PropertyEdit *>(editor);
    if(e) {
        return e->data();
    }
    return QVariant();
}

bool Property::setEditorData(QWidget *editor, const QVariant &data) {
    PropertyEdit *e = dynamic_cast<PropertyEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setData(data);
        e->blockSignals(false);
        return true;
    }
    return false;
}

bool Property::isCheckable() const {
    return m_checkable;
}

bool Property::isChecked() const {
    Actions *actions = dynamic_cast<Actions *>(m_editor);
    if(actions) {
        return static_cast<Actions *>(m_editor)->isChecked();
    }

    return false;
}

void Property::setChecked(bool value) {
    Actions *actions = dynamic_cast<Actions *>(m_editor);
    if(actions) {
        actions->onDataChanged(value);
    }
}

void Property::onDataChanged() {
    PropertyEdit *e = dynamic_cast<PropertyEdit *>(m_editor);
    if(e) {
        setValue(e->data());
    }
}

void Property::onEditorDestoyed() {
    m_editor = nullptr;
}
