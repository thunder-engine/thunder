#include "createcomponent.h"

#include <components/component.h>

CreateComponent::CreateComponent(const TString &type, Object *object, ObjectController *ctrl, UndoCommand *group) :
        UndoCommand(QObject::tr("Create %1").arg(type.data()).toStdString(), group),
        m_type(type),
        m_controller(ctrl),
        m_object(object->uuid()),
        m_component(0) {

}

void CreateComponent::undo() {
    if(m_component) {
        Object *object = Engine::findObject(m_component);
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
            m_component = component->uuid();

            emit m_controller->objectsSelected({Engine::findObject(m_object)});
        }
    }
}
