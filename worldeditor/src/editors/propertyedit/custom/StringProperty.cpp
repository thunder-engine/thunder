#include "StringProperty.h"

#include "../editors/StringEdit.h"
#include "../nextobject.h"

StringProperty::StringProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {
}

StringProperty::~StringProperty() {

}

QWidget *StringProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &option) {
    Q_UNUSED(option);
    m_editor = new StringEdit(parent);
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_editor->setDisabled(object->isReadOnly(objectName()));
    }
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

QSize StringProperty::sizeHint(const QSize& size) const {
    return QSize(size.width(), 26);
}

void StringProperty::onDataChanged() {
    setValue(QVariant(static_cast<StringEdit *>(m_editor)->text()));
}
