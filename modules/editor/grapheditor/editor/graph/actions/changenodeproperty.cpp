#include "changenodeproperty.h"

ChangeNodeProperty::ChangeNodeProperty(const Object::ObjectList &objects, const std::string &property, const Variant &value, GraphController *ctrl, const QString &name, QUndoCommand *group) :
        UndoCommand(name, ctrl, group),
        m_value(value),
        m_property(property),
        m_controller(ctrl) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }

}

void ChangeNodeProperty::undo() {
    ChangeNodeProperty::redo();
}

void ChangeNodeProperty::redo() {
    Object::ObjectList objects;

    Variant value(m_value);

    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            m_value = object->property(m_property.c_str());
            object->setProperty(m_property.c_str(), value);

            objects.push_back(object);
        }
    }

    auto g = m_controller->graph();

    g->emitSignal(_SIGNAL(graphUpdated()));
}
