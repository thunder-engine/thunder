#include "deletewiget.h"

DeleteObject::DeleteObject(const Object::ObjectList &objects, WidgetController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_controller(ctrl) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }
}

void DeleteObject::undo() {
    m_objects.clear();

    for(auto &ref : m_dump) {
        Object *object = Engine::toObject(ref);
        if(object) {
            m_objects.push_back(object->uuid());
        }
    }

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    emit m_controller->sceneUpdated();
}

void DeleteObject::redo() {
    m_dump.clear();

    for(auto it : m_objects)  {
        Object *object = Engine::findObject(it);
        if(object) {
            m_dump.push_back(Engine::toVariant(object));
            delete object;
        }
    }

    m_controller->clear(true);

    emit m_controller->sceneUpdated();
}
