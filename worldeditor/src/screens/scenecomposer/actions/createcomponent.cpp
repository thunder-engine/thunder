#include "createcomponent.h"

#include <components/component.h>

CreateComponent::CreateComponent(const std::string &type, Object *object, ObjectController *ctrl, QUndoCommand *group) :
        UndoCommand(QObject::tr("Create %1").arg(type.c_str()), ctrl, group),
        m_type(type),
        m_controller(ctrl),
        m_object(object->uuid()) {

}

void CreateComponent::undo() {
    for(auto uuid : m_objects) {
        Object *object = Engine::findObject(uuid);
        if(object) {
            delete object;

            emit m_controller->objectsSelected({Engine::findObject(m_object)});
        }
    }
}

void CreateComponent::redo() {
    Object *parent = Engine::findObject(m_object);

    if(parent) {
        Component *component = dynamic_cast<Component *>(Engine::objectCreate(m_type, m_type, parent));
        if(component) {
            component->composeComponent();
            m_objects.push_back(component->uuid());

            emit m_controller->objectsSelected({Engine::findObject(m_object)});
        }
    }
}
