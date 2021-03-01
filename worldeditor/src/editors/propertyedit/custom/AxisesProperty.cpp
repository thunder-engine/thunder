#include "AxisesProperty.h"

#include "../editors/AxisesEdit.h"
#include "../nextobject.h"

AxisesProperty::AxisesProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *AxisesProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &option) {
    Q_UNUSED(option)
    m_Editor = new AxisesEdit(parent);
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_Editor->setDisabled(object->isReadOnly(objectName()));
    }
    connect(m_Editor, SIGNAL(axisesChanged(int)), this, SLOT(onDataChanged(int)));

    return m_Editor;
}

bool AxisesProperty::setEditorData(QWidget *editor, const QVariant &data) {
    AxisesEdit *e = dynamic_cast<AxisesEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setAxises(data.toInt());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant AxisesProperty::editorData(QWidget *editor) {
    AxisesEdit *e = dynamic_cast<AxisesEdit *>(editor);
    if(e) {
        return QVariant(e->axises());
    }
    return Property::editorData(editor);
}

void AxisesProperty::onDataChanged(int data) {
    setValue(QVariant(data));
}

