#include <QPainter>

#include <engine.h>

#include "ComponentProperty.h"

#include "editors/componentselect/componentselect.h"
#include "../nextobject.h"

ComponentProperty::ComponentProperty(const QString &name, QObject *propertyObject, QObject *parent) :
        Property(name, propertyObject, parent) {

}

QWidget *ComponentProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &) {
    ComponentSelect *editor = new ComponentSelect(parent);
    m_editor = editor;
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        m_editor->setDisabled(object->isReadOnly(objectName()));
    }
    connect(editor, &ComponentSelect::componentChanged, this, &ComponentProperty::onComponentChanged);
    return editor;
}

bool ComponentProperty::setEditorData(QWidget *editor, const QVariant &data) {
    ComponentSelect *e = static_cast<ComponentSelect *>(editor);
    if(e) {
        e->blockSignals(true);
        e->setData(data.value<SceneComponent>());
        e->blockSignals(false);
        return true;
    }
    return Property::setEditorData(editor, data);
}

QVariant ComponentProperty::editorData(QWidget *editor) {
    ComponentSelect *e = static_cast<ComponentSelect *>(editor);
    if(e) {
        return QVariant::fromValue(e->data());
    }
    return Property::editorData(editor);
}

QSize ComponentProperty::sizeHint(const QSize &size) const {
    return QSize(size.width(), 26);
}

void ComponentProperty::onComponentChanged(const SceneComponent &component) {
    setValue(QVariant::fromValue(component));
}
