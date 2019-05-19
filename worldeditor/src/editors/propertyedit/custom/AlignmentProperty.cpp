#include "AlignmentProperty.h"

#include "../editors/AlignmentEdit.h"

AlignmentProperty::AlignmentProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *AlignmentProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &option) {
    Q_UNUSED(option)
    m_Editor = new AlignmentEdit(parent);
    connect(m_Editor, SIGNAL(alignmentChanged(int)), this, SLOT(onDataChanged(int)));
    return m_Editor;
}

bool AlignmentProperty::setEditorData(QWidget *editor, const QVariant &data) {
    AlignmentEdit *e = dynamic_cast<AlignmentEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setAlignment(static_cast<AlignmentEdit::Alignment>(data.toInt()));
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant AlignmentProperty::editorData(QWidget *editor) {
    AlignmentEdit *e = dynamic_cast<AlignmentEdit *>(editor);
    if(e) {
        return QVariant(e->alignment());
    }
    return Property::editorData(editor);
}

void AlignmentProperty::onDataChanged(int data) {
    setValue(QVariant(data));
}
