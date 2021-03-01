#include "FloatProperty.h"

#include "../editors/FloatEdit.h"
#include "../nextobject.h"

FloatProperty::FloatProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *FloatProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &option) {
    Q_UNUSED(option)
    m_Editor = new FloatEdit(parent);
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_Editor->setDisabled(object->isReadOnly(objectName()));
    }
    connect(m_Editor, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
    return m_Editor;
}

bool FloatProperty::setEditorData(QWidget *editor, const QVariant &data) {
    FloatEdit *e = dynamic_cast<FloatEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setValue(data.toDouble());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant FloatProperty::editorData(QWidget *editor) {
    FloatEdit *e = dynamic_cast<FloatEdit *>(editor);
    if(e) {
        return QVariant(e->value());
    }
    return Property::editorData(editor);
}

void FloatProperty::onDataChanged() {
    FloatEdit *e = dynamic_cast<FloatEdit *>(m_Editor);
    if(e) {
        setValue(QVariant(e->value()));
    }
}

QSize FloatProperty::sizeHint(const QSize &size) const {
    return QSize(size.width(), 27);
}
