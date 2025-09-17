#include "createobjectserial.h"

#include <components/actor.h>
#include <components/transform.h>

#include <resources/prefab.h>

#include <log.h>

CreateObjectSerial::CreateObjectSerial(const TString &ref, const Vector3 &position, uint32_t parent, ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_reference(ref),
        m_position(position),
        m_controller(ctrl),
        m_parent(parent) {

}

void CreateObjectSerial::undo() {
    std::set<Scene *> scenes;
    for(auto &it : m_controller->selected()) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor) {
            scenes.insert(actor->scene());
        }
        delete it;
    }

    m_controller->clear(false);
    m_controller->selectActors(m_selectedObjects);

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }
}

void CreateObjectSerial::redo() {
    m_selectedObjects.clear();
    for(auto &it : m_controller->selected()) {
        m_selectedObjects.push_back(it->uuid());
    }

    Scene *scene = nullptr;

    std::list<uint32_t> objects;

    Resource *resource = Engine::loadResource<Resource>(m_reference);
    if(resource) {
        Prefab *prefab = dynamic_cast<Prefab *>(resource);
        if(prefab) {
            Actor *clone = static_cast<Actor *>(prefab->actor()->clone(Engine::findObject(m_parent)));
            scene = clone->scene();
            clone->transform()->setPosition(m_position);

            Object::ObjectList list;
            Engine::enumObjects(clone, list);

            if(m_staticIds.empty()) {
                for(auto it : list) {
                    m_staticIds[it->clonedFrom()] = it->uuid();
                }
            } else {
                for(auto it : list) {
                    auto staticIt = m_staticIds.find(it->clonedFrom());
                    if(staticIt != m_staticIds.end()) {
                        Engine::replaceUUID(it, staticIt->second);
                    }
                }
            }

            objects.push_back(clone->uuid());
        }
    } else {
        aWarning() << "Broken object";
    }

    m_controller->clear(false);
    m_controller->selectActors(objects);

    emit m_controller->sceneUpdated(scene);
}
