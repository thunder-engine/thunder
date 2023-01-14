#include "objectproperty.h"

#include "objectselect.h"

ObjectProperty::ObjectProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *ObjectProperty::createEditor(QWidget *parent) const {
    ObjectSelect *editor = new ObjectSelect(parent);
    m_editor = editor;
    m_editor->setDisabled(isReadOnly());

    connect(editor, &ObjectSelect::valueChanged, this, &ObjectProperty::onValueChanged);
    return editor;
}

bool ObjectProperty::setEditorData(QWidget *editor, const QVariant &data) {
    ObjectSelect *e = static_cast<ObjectSelect *>(editor);
    if(e) {
        e->blockSignals(true);
        if(QString(data.typeName()) == "ObjectData") {
            e->setObjectData(data.value<ObjectData>());
        } else if(QString(data.typeName()) == "Template") {
            e->setTemplateData(data.value<Template>());
        }
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant ObjectProperty::editorData(QWidget *editor) {
    ObjectSelect *e = static_cast<ObjectSelect *>(editor);
    if(e) {
        return e->data();
    }
    return Property::editorData(editor);
}

void ObjectProperty::onValueChanged() {
    setValue(m_editor ? static_cast<ObjectSelect *>(m_editor)->data() : QVariant());
}
