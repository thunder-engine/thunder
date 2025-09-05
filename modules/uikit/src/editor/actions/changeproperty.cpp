#include "changeproperty.h"

ChangeProperty::ChangeProperty(const Object::ObjectList &objects, const TString &property, const Variant &value, WidgetController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_value(value),
        m_property(property),
        m_controller(ctrl) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }
}

void ChangeProperty::undo() {
    ChangeProperty::redo();
}

void ChangeProperty::redo() {
    std::list<Object *> objects;

    Variant value(m_value);

    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            m_value = object->property(m_property.data());
            object->setProperty(m_property.data(), value);

            objects.push_back(object);
        }
    }

    if(!objects.empty()) {
        emit m_controller->propertyChanged(objects, m_property, value);
    }
}
