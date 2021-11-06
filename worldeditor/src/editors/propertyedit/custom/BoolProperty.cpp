#include "BoolProperty.h"

#include "../editors/BooleanEdit.h"

BoolProperty::BoolProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *BoolProperty::createEditor(QWidget *parent) const {
    m_editor = new BooleanEdit(parent);
    m_editor->setDisabled(isReadOnly());

    connect(m_editor, SIGNAL(stateChanged(int)), this, SLOT(onDataChanged(int)));
    return m_editor;
}

bool BoolProperty::setEditorData(QWidget *editor, const QVariant &data) {
    BooleanEdit *e = static_cast<BooleanEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setValue(data.toBool());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant BoolProperty::editorData(QWidget *editor) {
    BooleanEdit *e = static_cast<BooleanEdit *>(editor);
    if(e) {
        return QVariant(e->value());
    }
    return Property::editorData(editor);
}

void BoolProperty::onDataChanged(int data) {
    setValue(QVariant((data != Qt::Unchecked)));
}
