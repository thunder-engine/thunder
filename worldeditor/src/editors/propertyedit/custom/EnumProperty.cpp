#include "EnumProperty.h"

#include <QMetaObject>
#include <QMetaEnum>
#include <QMetaProperty>

#include "../editors/ComboEdit.h"
#include "../nextobject.h"

EnumProperty::EnumProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {
    const QMetaObject *meta = propertyObject->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(qPrintable(name)));

    if(prop.isEnumType()) {
        QMetaEnum qenum = prop.enumerator();
        for(int i = 0; i < qenum.keyCount(); i++) {
            m_enum << qenum.key(i);
        }
    }
}

QVariant EnumProperty::value(int role) const {
    if(role == Qt::DisplayRole) {
        if (m_propertyObject) {
            int index = m_propertyObject->property(qPrintable(objectName())).toInt();

            const QMetaObject *meta = m_propertyObject->metaObject();
            QMetaProperty prop = meta->property(meta->indexOfProperty(qPrintable(objectName())));
            return QVariant(prop.enumerator().valueToKey(index));
        } else {
            return QVariant();
        }
    } else {
        return Property::value(role);
    }
}

QWidget *EnumProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &) {
    ComboEdit *editor = new ComboEdit(parent);
    editor->addItems(m_enum);

    m_editor = editor;
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_editor->setDisabled(object->isReadOnly(objectName()));
    }

    connect(m_editor, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(valueChanged(const QString &)));

    return m_editor;
}

bool EnumProperty::setEditorData(QWidget *editor, const QVariant &data) {
    ComboEdit *e = static_cast<ComboEdit *>(editor);
    if(e) {
        int value = data.toInt();
        const QMetaObject *meta = m_propertyObject->metaObject();
        QMetaProperty prop = meta->property(meta->indexOfProperty(qPrintable(objectName())));

        int index = e->findText(prop.enumerator().valueToKey(value));
        if(index == -1) {
            return false;
        }
        e->setCurrentIndex(index);
    } else {
        return false;
    }

    return true;
}

QVariant EnumProperty::editorData(QWidget *editor) {
    ComboEdit *e = static_cast<ComboEdit *>(editor);
    if(e) {
        return QVariant(e->currentText());
    } else {
        return QVariant();
    }
}

void EnumProperty::valueChanged(const QString &item) {
    setValue(QVariant(item));
}
