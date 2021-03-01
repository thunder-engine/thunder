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
    m_Editor = new StringEdit(parent);
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_Editor->setDisabled(object->isReadOnly(objectName()));
    }
    connect(m_Editor, SIGNAL(editFinished()), this, SLOT(onDataChanged()));
    return m_Editor;
}

bool StringProperty::setEditorData(QWidget *editor, const QVariant &data) {
    StringEdit *e = dynamic_cast<StringEdit *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setText(data.toString());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant StringProperty::editorData(QWidget *editor) {
    StringEdit *e = dynamic_cast<StringEdit *>(editor);
    if(e) {
        return QVariant(e->text());
    }
    return Property::editorData(editor);
}

QSize StringProperty::sizeHint(const QSize& size) const {
    return QSize(size.width(), 26);
}

void StringProperty::onDataChanged() {
    setValue(QVariant(static_cast<StringEdit *>(m_Editor)->text()));
}
