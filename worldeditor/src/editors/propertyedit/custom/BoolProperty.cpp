#include "BoolProperty.h"

#include <QCheckBox>

#include "../nextobject.h"

BoolProperty::BoolProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *BoolProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &option) {
    Q_UNUSED(option)
    m_editor = new QCheckBox(parent);
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_editor->setDisabled(object->isReadOnly(objectName()));
    }
    connect(m_editor, SIGNAL(stateChanged(int)), this, SLOT(onDataChanged(int)));
    return m_editor;
}

bool BoolProperty::setEditorData(QWidget *editor, const QVariant &data) {
    QCheckBox *e = static_cast<QCheckBox *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setChecked(data.toBool());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant BoolProperty::editorData(QWidget *editor) {
    QCheckBox *e = static_cast<QCheckBox *>(editor);
    if(e) {
        return QVariant(e->isChecked());
    }
    return Property::editorData(editor);
}

void BoolProperty::onDataChanged(int data) {
    setValue(QVariant((data != Qt::Unchecked)));
}
