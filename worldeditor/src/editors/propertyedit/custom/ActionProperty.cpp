#include "ActionProperty.h"

#include "../editors/Actions.h"

#include "../nextobject.h"

ActionProperty::ActionProperty(const QString &name, QObject *propertyObject, QObject *parent, bool root) :
        Property(name, propertyObject, parent, root) {

    m_checkable = m_root && (static_cast<NextObject *>(m_propertyObject) != nullptr);
}

bool ActionProperty::isReadOnly() const {
    NextObject *object = dynamic_cast<NextObject *>(m_propertyObject);
    if(object) {
        return object->isReadOnly(objectName());
    }
    return Property::isReadOnly();
}

QWidget *ActionProperty::createEditor(QWidget *parent) const {
    NextObject *next = static_cast<NextObject *>(m_propertyObject);
    Actions *act = new Actions(m_name, parent);
    Object *object = next->component(objectName());
    act->setObject(object);
    act->setMenu(next->menu(object));

    m_editor = act;

    return m_editor;
}

bool ActionProperty::isChecked() const {
    if(m_editor) {
        return static_cast<Actions *>(m_editor)->isChecked();
    }
    return false;
}

void ActionProperty::setChecked(bool value) {
    if(m_editor) {
        static_cast<Actions *>(m_editor)->onDataChanged(value);
    }
}
