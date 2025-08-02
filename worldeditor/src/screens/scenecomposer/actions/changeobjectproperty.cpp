#include "changeobjectproperty.h"

#include <components/actor.h>
#include <components/component.h>

ChangeObjectProperty::ChangeObjectProperty(const Object::ObjectList &objects, const TString &property, const Variant &value, ObjectController *ctrl, const QString &name, QUndoCommand *group) :
        UndoCommand(name, ctrl, group),
        m_value(value),
        m_property(property),
        m_controller(ctrl) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }

}

void ChangeObjectProperty::undo() {
    ChangeObjectProperty::redo();
}

void ChangeObjectProperty::redo() {
    std::set<Scene *> scenes;
    Object::ObjectList objects;

    Variant value(m_value);

    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            m_value = object->property(m_property.data());
            object->setProperty(m_property.data(), value);

            objects.push_back(object);

            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            } else {
                Component *component = dynamic_cast<Component *>(object);
                if(component) {
                    scenes.insert(component->actor()->scene());
                }
            }
        }
    }

    if(!objects.empty()) {
        emit m_controller->propertyChanged(objects, m_property, value);
    }

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }
}
