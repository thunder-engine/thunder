#include "createobjectserial.h"

#include <components/actor.h>
#include <log.h>

CreateObjectSerial::CreateObjectSerial(Object::ObjectList &list, ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_controller(ctrl) {

    for(auto it : list) {
        m_dump.push_back(ObjectSystem::toVariant(it));
        m_parents.push_back(it->parent()->uuid());
        delete it;
    }
}
void CreateObjectSerial::undo() {
    QSet<Scene *> scenes;
    for(auto &it : m_controller->selected()) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor) {
            scenes.insert(actor->scene());
        }
        delete it;
    }

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }
}
void CreateObjectSerial::redo() {
    m_objects.clear();
    for(auto &it : m_controller->selected()) {
        m_objects.push_back(it->uuid());
    }
    auto it = m_parents.begin();

    Scene *scene = nullptr;

    std::list<uint32_t> objects;
    for(auto &ref : m_dump) {
        Object *object = Engine::toObject(ref);
        if(object) {
            object->setParent(Engine::findObject(*it));
            objects.push_back(object->uuid());
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scene = actor->scene();
            }
        } else {
            aWarning() << "Broken object";
        }
        ++it;
    }

    m_controller->clear(false);
    m_controller->selectActors(objects);

    emit m_controller->sceneUpdated(scene);
}
