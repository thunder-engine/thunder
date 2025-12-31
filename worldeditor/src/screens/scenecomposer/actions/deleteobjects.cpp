#include "deleteobjects.h"

#include <components/actor.h>
#include <components/component.h>

#include <set>

DeleteObjects::DeleteObjects(const Object::ObjectList &objects, ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_controller(ctrl) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }
}

void DeleteObjects::undo() {
    std::set<Scene *> scenes;

    auto it = m_parents.begin();
    auto index = m_indices.begin();
    for(auto &ref : m_dump) {
        Object *parent = Engine::findObject(*it);
        Object *object = Engine::toObject(ref, parent);
        if(object) {
            object->setParent(parent, *index);
            m_objects.push_back(object->uuid());
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }
        }
        ++it;
        ++index;
    }

    if(!m_objects.empty()) {
        auto it = m_objects.begin();
        while(it != m_objects.end()) {
            Component *comp = dynamic_cast<Component *>(Engine::findObject(*it));
            if(comp) {
                *it = comp->parent()->uuid();
            }
            ++it;
        }
        m_controller->clear(false);
        m_controller->selectActors(m_objects);
    }

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }
}

void DeleteObjects::redo() {
    std::set<Scene *> scenes;

    m_parents.clear();
    m_dump.clear();
    for(auto it : m_objects)  {
        Object *object = Engine::findObject(it);
        if(object) {
            m_dump.push_back(Engine::toVariant(object));
            m_parents.push_back(object->parent()->uuid());

            int index = -1;
            for(auto it : object->parent()->getChildren()) {
                index++;
                if(it == object) {
                    break;
                }
            }
            if(index != -1) {
                m_indices.push_back(index);
            }
        }
    }
    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }
            delete object;
        }
    }
    m_objects.clear();

    m_controller->clear();

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }
}
