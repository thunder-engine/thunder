#include "removecomponent.h"

#include <components/actor.h>
#include <components/component.h>

RemoveComponent::RemoveComponent(const TString &component, ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name + " " + component, group),
        m_controller(ctrl),
        m_parent(0),
        m_uuid(0),
        m_index(0) {

    for(auto it : m_controller->selected()) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor) {
            Component *comp = actor->component(component);
            if(comp) {
                m_uuid = comp->uuid();
            }
        }
    }
}

void RemoveComponent::undo() {
    Object *parent = Engine::findObject(m_parent);
    Object *object = Engine::toObject(m_dump, parent);
    if(object) {
        object->setParent(parent, m_index);

        emit m_controller->objectsSelected({parent});
    }
}

void RemoveComponent::redo() {
    m_dump = Variant();
    m_parent = 0;
    Object *object = Engine::findObject(m_uuid);
    if(object) {
        m_dump = Engine::toVariant(object, true);

        Object *parent = object->parent();

        m_parent = parent->uuid();

        auto &list = parent->getChildren();
        QList<Object *> children(list.begin(), list.end());
        m_index = children.indexOf(object);

        delete object;

        emit m_controller->objectsSelected({parent});
    }
}
