#include "pasteobjects.h"

#include <components/world.h>
#include <components/actor.h>
#include <components/component.h>
#include <components/transform.h>

#include <set>

PasteObjects::PasteObjects(const VariantList &data, Object *parent, ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_data(data),
        m_controller(ctrl),
        m_parent(parent->uuid()),
        m_prefab(0) {

    Prefab *fab = m_controller->isolatedPrefab();
    if(fab) {
        m_prefab = fab->uuid();
    }
}

void PasteObjects::undo() {
    std::set<Scene *> scenes;

    for(auto it : m_uuidPairs) {
        Object *object = Engine::findObject(it.second);
        if(object) {
            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            }

            delete object;
        }
    }

    m_controller->clear();
    m_controller->selectActors(m_selected);

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }

    if(m_prefab) {
        emit m_controller->sceneUpdated(Engine::findObject(m_prefab));
    }
}

void PasteObjects::redo() {
    std::set<Scene *> scenes;

    std::list<uint32_t> list;

    bool generateStatics = m_uuidPairs.empty();
    Object *parent = Engine::findObject(m_parent);

    for(auto &dataIt : m_data) {
        Object *object = buildObject(dataIt, parent, generateStatics);

        if(m_prefab) {
            Object::ObjectList clones;
            m_controller->getClones(clones, parent->uuid(), Engine::world());
            for(auto clone : clones) {
                buildObject(dataIt, clone, generateStatics);
            }
        }

        Actor *actor = dynamic_cast<Actor *>(object);
        if(actor) {
            scenes.insert(actor->scene());
            list.push_back(actor->uuid());
        } else {
            Component *component = dynamic_cast<Component *>(object);
            if(component) {
                scenes.insert(component->scene());
            }
        }
    }

    m_selected.clear();
    for(auto it : m_controller->selected()) {
        m_selected.push_back(it->uuid());
    }

    m_controller->clear();
    m_controller->selectActors(list);

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }

    if(m_prefab) {
        emit m_controller->sceneUpdated(Engine::findObject(m_prefab));
    }
}

Object *PasteObjects::buildObject(const Variant &data, Object *parent, bool generate) {
    Engine::blockObjectCache(true);
    Object *object = Engine::toObject(data, parent);
    Engine::blockObjectCache(false);

    Object::ObjectList objects;
    Object::enumObjects(object, objects);
    for(auto it : objects) {
        uint32_t oldUuid = it->uuid();
        uint32_t newUuid = 0;
        if(generate) {
            newUuid = Engine::generateUUID();
            m_uuidPairs[oldUuid] = newUuid;
        } else {
            newUuid = m_uuidPairs[oldUuid];
        }
        detachObjectUUID(it, newUuid);
    }

    return object;
}

void PasteObjects::detachObjectUUID(Object *object, uint32_t uuid) {
    // To detach from initial uuid
    Engine::blockObjectCache(true);
    Engine::replaceUUID(object, uuid);
    Engine::blockObjectCache(false);

    Engine::replaceUUID(object, uuid);
}
