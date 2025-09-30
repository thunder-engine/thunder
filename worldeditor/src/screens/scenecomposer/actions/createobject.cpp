#include "createobject.h"

#include <components/scene.h>
#include <components/actor.h>

CreateObject::CreateObject(const TString &type, Scene *scene, ObjectController *ctrl, UndoCommand *group) :
        UndoCommand(QObject::tr("Create %1").arg(type.data()).toStdString(), group),
        m_type(type),
        m_controller(ctrl),
        m_scene(scene->uuid()) {

}

void CreateObject::undo() {
    std::set<Scene *> scenes;

    for(auto uuid : m_objects) {
        Object *object = Engine::findObject(uuid);
        if(object) {
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }

            delete object;
        }
    }

    m_controller->clear(false);
    m_controller->selectActors(m_selected);

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }
}

void CreateObject::redo() {
    std::set<Scene *> scenes;

    m_objects.clear();
    m_selected.clear();

    for(auto it : m_controller->selected()) {
        m_selected.push_back(it->uuid());
    }

    Object *parent = Engine::findObject(m_scene);
    TString component = (m_type == "Actor") ? "" : m_type;
    Object *object = Engine::composeActor(component, m_controller->findFreeObjectName(m_type, parent), parent);

    if(object) {
        Actor *actor = dynamic_cast<Actor *>(object);
        if(actor) {
            scenes.insert(actor->scene());
        }

        m_objects.push_back(object->uuid());
    }


    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }
}
