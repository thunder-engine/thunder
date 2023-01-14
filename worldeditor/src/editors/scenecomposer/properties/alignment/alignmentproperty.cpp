#include "alignmentproperty.h"

#include "alignmentedit.h"

AlignmentProperty::AlignmentProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *AlignmentProperty::createEditor(QWidget *parent) const {
    m_editor = new AlignmentEdit(parent);
    m_editor->setDisabled(isReadOnly());

    connect(m_editor, SIGNAL(alignmentChanged(int)), this, SLOT(onDataChanged(int)));
    return m_editor;
}

bool AlignmentProperty::setEditorData(QWidget *editor, const QVariant &data) {
    AlignmentEdit *e = static_cast<AlignmentEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setAlignment(static_cast<AlignmentEdit::Alignment>(data.toInt()));
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant AlignmentProperty::editorData(QWidget *editor) {
    AlignmentEdit *e = static_cast<AlignmentEdit *>(editor);
    if(e) {
        return QVariant(e->alignment());
    }
    return Property::editorData(editor);
}

void AlignmentProperty::onDataChanged(int data) {
    setValue(QVariant(data));
}
