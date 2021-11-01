#include "AxisesProperty.h"

#include "../editors/AxisesEdit.h"

AxisesProperty::AxisesProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *AxisesProperty::createEditor(QWidget *parent) const {
    m_editor = new AxisesEdit(parent);
    m_editor->setDisabled(isReadOnly());

    connect(m_editor, SIGNAL(axisesChanged(int)), this, SLOT(onDataChanged(int)));

    return m_editor;
}

bool AxisesProperty::setEditorData(QWidget *editor, const QVariant &data) {
    AxisesEdit *e = static_cast<AxisesEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setAxises(data.toInt());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant AxisesProperty::editorData(QWidget *editor) {
    AxisesEdit *e = static_cast<AxisesEdit *>(editor);
    if(e) {
        return QVariant(e->axises());
    }
    return Property::editorData(editor);
}

void AxisesProperty::onDataChanged(int data) {
    setValue(QVariant(data));
}
