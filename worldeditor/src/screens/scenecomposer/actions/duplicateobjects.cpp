#include "duplicateobjects.h"

#include <components/actor.h>
#include <components/scene.h>

DuplicateObjects::DuplicateObjects(ObjectController *ctrl, const QString &name, QUndoCommand *group) :
        UndoCommand(name, ctrl, group),
        m_controller(ctrl) {

}

void DuplicateObjects::undo() {
    Scene *scene = nullptr;
    m_dump.clear();
    for(auto it : m_objects) {
        Actor *actor = dynamic_cast<Actor *>(Engine::findObject(it));
        if(actor) {
            VariantList pair;
            scene = actor->scene();
            pair.push_back(scene->uuid());
            pair.push_back(ObjectSystem::toVariant(actor));

            m_dump.push_back(pair);
            delete actor;
        }
    }
    m_objects.clear();

    m_controller->clear(false);
    m_controller->selectActors(m_selected);

    emit m_controller->sceneUpdated(scene);
}

void DuplicateObjects::redo() {
    Scene *scene = nullptr;
    if(m_dump.empty()) {
        for(auto &it : m_controller->selected()) {
            m_selected.push_back(it->uuid());
            Actor *actor = dynamic_cast<Actor *>(it->clone(it->parent()));
            if(actor) {
                static_cast<Object *>(actor)->clearCloneRef();
                actor->setName(ObjectController::findFreeObjectName(it->name(), it->parent()));
                m_objects.push_back(actor->uuid());
                scene = actor->scene();
            }
        }
    } else {
        for(auto &it : m_dump) {
            VariantList pair = it.toList();

            scene = dynamic_cast<Scene *>(Engine::findObject(pair.front().toInt()));
            Object *obj = ObjectSystem::toObject(pair.back(), scene);
            if(obj) {
                m_objects.push_back(obj->uuid());
            }
        }
    }

    m_controller->clear(false);
    m_controller->selectActors(m_objects);

    emit m_controller->sceneUpdated(scene);
}
