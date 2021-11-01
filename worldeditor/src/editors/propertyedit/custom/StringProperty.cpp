#include "StringProperty.h"

#include "../editors/StringEdit.h"

StringProperty::StringProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *StringProperty::createEditor(QWidget *parent) const {
    m_editor = new StringEdit(parent);
    m_editor->setDisabled(isReadOnly());
    connect(m_editor, SIGNAL(editFinished()), this, SLOT(onDataChanged()));
    return m_editor;
}

bool StringProperty::setEditorData(QWidget *editor, const QVariant &data) {
    StringEdit *e = static_cast<StringEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setText(data.toString());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant StringProperty::editorData(QWidget *editor) {
    StringEdit *e = static_cast<StringEdit *>(editor);
    if(e) {
        return QVariant(e->text());
    }
    return Property::editorData(editor);
}

void StringProperty::onDataChanged() {
    setValue(QVariant(static_cast<StringEdit *>(m_editor)->text()));
}
