#include "NextEnumProperty.h"

#include <metaobject.h>
#include <metaenum.h>
#include <metaproperty.h>

#include <QDebug>

#include "../editors/ComboEdit.h"
#include "../nextobject.h"

NextEnumProperty::NextEnumProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent),
        m_metaEnum(MetaEnum(nullptr)) {

}

QVariant NextEnumProperty::value(int role) const {
    if(role == Qt::DisplayRole) {
        if(m_propertyObject) {
            return QVariant::fromValue(m_Value);
        } else {
            return QVariant();
        }
    }
    return Property::value(role);
}

QWidget *NextEnumProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &) {
    ComboEdit *editor = new ComboEdit(parent);
    editor->addItems(m_enum);

    m_Editor = editor;
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_Editor->setDisabled(object->isReadOnly(objectName()));
    }

    connect(m_Editor, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(valueChanged(const QString &)));

    return m_Editor;
}

bool NextEnumProperty::setEditorData(QWidget *editor, const QVariant &data) {
    ComboEdit *e = dynamic_cast<ComboEdit *>(editor);
    if(e) {
        Enum value = data.value<Enum>();
        if(value.m_Object) {
            m_Value = value;
            const MetaObject *meta = m_Value.m_Object->metaObject();
            int index = meta->indexOfEnumerator(qPrintable(m_Value.m_EnumName));
            if(index > -1) {
                m_metaEnum = meta->enumerator(index);
                m_enum.clear();
                int idx = 0;
                for(int i = 0; i < m_metaEnum.keyCount(); i++) {
                    m_enum << m_metaEnum.key(i);
                    if(m_metaEnum.value(i) == m_Value.m_Value) {
                        idx = i;
                    }
                }

                e->blockSignals(true);
                e->clear();
                e->addItems(m_enum);
                e->setCurrentIndex(idx);
                e->blockSignals(false);
            }
        } else {
            qDebug() << "Empty data provided";
        }
    } else {
        return false;
    }

    return true;
}

QVariant NextEnumProperty::editorData(QWidget *editor) {
    return QVariant(m_Value.m_Value);
}

void NextEnumProperty::valueChanged(const QString &item) {
    int idx = m_Value.m_Value;
    for(int i = 0; i < m_metaEnum.keyCount(); i++) {
        if(item == m_metaEnum.key(i)) {
            idx = i;
            break;
        }
    }
    m_Value.m_Value = idx;
    setValue(QVariant::fromValue(m_Value));
}
