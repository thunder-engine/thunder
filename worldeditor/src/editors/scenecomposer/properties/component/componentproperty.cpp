#include "componentproperty.h"

#include "actions.h"

#include "../../nextobject.h"

#include <invalid.h>

ComponentProperty::ComponentProperty(const QString &name, QObject *propertyObject, QObject *parent, bool root) :
        Property(name, propertyObject, parent, root) {

    m_object = static_cast<NextObject *>(m_propertyObject)->component(objectName());
    Invalid *invalid = dynamic_cast<Invalid *>(m_object);
    if(invalid) {
        m_name += " (Invalid)";
    }

    m_checkable = m_root && (static_cast<NextObject *>(m_propertyObject) != nullptr);
}

bool ComponentProperty::isReadOnly() const {
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        return object->isReadOnly(objectName());
    }
    return Property::isReadOnly();
}

QWidget *ComponentProperty::createEditor(QWidget *parent) const {
    Actions *act = new Actions(m_name, parent);
    act->setObject(m_object);
    act->setMenu(static_cast<NextObject *>(m_propertyObject)->menu(m_object));

    m_editor = act;

    return m_editor;
}

bool ComponentProperty::isChecked() const {
    if(m_editor) {
        return static_cast<Actions *>(m_editor)->isChecked();
    }
    return false;
}

void ComponentProperty::setChecked(bool value) {
    if(m_editor) {
        static_cast<Actions *>(m_editor)->onDataChanged(value);
    }
}
