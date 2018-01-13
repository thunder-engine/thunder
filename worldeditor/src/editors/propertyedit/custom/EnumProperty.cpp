#include "EnumProperty.h"

#include <QMetaObject>
#include <QMetaEnum>
#include <QMetaProperty>
#include <QComboBox>

EnumProperty::EnumProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {
    const QMetaObject *meta = propertyObject->metaObject();
    QMetaProperty prop      = meta->property(meta->indexOfProperty(qPrintable(name)));

    if(prop.isEnumType()) {
        QMetaEnum qenum = prop.enumerator();
        for(int i = 0; i < qenum.keyCount(); i++) {
            m_enum << qenum.key(i);
        }
    }
}

QVariant EnumProperty::value(int role) const {
    if(role == Qt::DisplayRole){
        if (m_propertyObject){
            int index = m_propertyObject->property(qPrintable(objectName())).toInt();

            const QMetaObject *meta = m_propertyObject->metaObject();
            QMetaProperty prop      = meta->property(meta->indexOfProperty(qPrintable(objectName())));
            return QVariant(prop.enumerator().valueToKey(index));
        } else {
            return QVariant();
        }
    } else {
        return Property::value(role);
    }
}

QWidget *EnumProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &) {
    QComboBox *editor = new QComboBox(parent);
    editor->addItems(m_enum);

    connect(editor, SIGNAL(currentIndexChanged(const QString)),
            this, SLOT(valueChanged(const QString)));
    return editor;
}

bool EnumProperty::setEditorData(QWidget *editor, const QVariant &data) {
    QComboBox *combo = 0;
    if(combo = qobject_cast<QComboBox*>(editor)) {
        int value = data.toInt();
        const QMetaObject *meta = m_propertyObject->metaObject();
        QMetaProperty prop = meta->property(meta->indexOfProperty(qPrintable(objectName())));

        int index = combo->findText(prop.enumerator().valueToKey(value));
        if(index == -1) {
            return false;
        }
        combo->blockSignals(true);
        combo->setCurrentIndex(index);
        combo->blockSignals(false);
    } else {
        return false;
    }

    return true;
}

QVariant EnumProperty::editorData(QWidget *editor) {
    QComboBox *combo = 0;
    if(combo = qobject_cast<QComboBox*>(editor)) {
        return QVariant(combo->currentText());
    } else {
        return QVariant();
    }
}

void EnumProperty::valueChanged(const QString &item) {
    setValue(QVariant(item));
}
