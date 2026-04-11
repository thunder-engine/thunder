#include "createobject.h"

#include <components/world.h>
#include <components/actor.h>
#include <components/component.h>
#include <components/transform.h>

#include <set>

CreateObject::CreateObject(const TString &type, Object *parent, const Vector3 &position, ObjectController *ctrl) :
        UndoCommand(TString("Create %1").arg(type)),
        m_type(type),
        m_position(position),
        m_controller(ctrl),
        m_parent(parent->uuid()),
        m_prefab(0) {

    Prefab *fab = m_controller->isolatedPrefab();
    if(fab) {
        m_prefab = fab->uuid();
    }
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

            if(m_prefab) {
                Object::ObjectList clones;
                m_controller->getClones(clones, object->uuid(), Engine::world());
                for(auto it : clones) {
                    delete it;
                }
            }

            delete object;
        }
    }

    m_controller->clear(false);
    m_controller->selectActors(m_selected);

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }

    if(m_prefab) {
        emit m_controller->sceneUpdated(Engine::findObject(m_prefab));
    }
}

void CreateObject::redo() {
    std::set<Scene *> scenes;

    m_objects.clear();
    m_selected.clear();

    for(auto it : m_controller->selected()) {
        m_selected.push_back(it->uuid());
    }

    std::list<uint32_t> list;

    Object *parent = Engine::findObject(m_parent);
    Object *object = createObject(parent);
    if(object) {
        Object::ObjectList children;
        Engine::enumObjects(object, children);

        bool generateStatics = m_staticIds.empty();
        resolveUUID(children, object, generateStatics);

        if(m_prefab) {
            Object::ObjectList clones;
            m_controller->getClones(clones, parent->uuid(), Engine::world());
            for(auto it : clones) {
                Object *clone = object->clone(it);
                if(clone) {
                    Object::ObjectList cloneList;
                    Engine::enumObjects(clone, cloneList);

                    resolveUUID(cloneList, clone, generateStatics);
                }
            }
        }

        Actor *actor = dynamic_cast<Actor *>(object);
        if(actor) {
            scenes.insert(actor->scene());
            actor->transform()->setPosition(m_position);
            list.push_back(actor->uuid());
        } else {
            Component *component = dynamic_cast<Component *>(object);
            if(component) {
                component->composeComponent();
                scenes.insert(component->scene());
                list.push_back(component->actor()->uuid());
            }
        }

        m_objects.push_back(object->uuid());
    }


    m_controller->clear(false);
    m_controller->selectActors(list);

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }

    if(m_prefab) {
        emit m_controller->sceneUpdated(Engine::findObject(m_prefab));
    }
}

Object *CreateObject::createObject(Object *parent) {
    Object *object = nullptr;
    TString type = (m_type == "Actor") ? "" : m_type;
    if(m_type.front() == 'c') {
        type = m_type.right(2);
        object = Engine::objectCreate(type, type, parent);
    } else if(type.front() == '{') {
        Prefab *prefab = Engine::loadResource<Prefab>(type);
        if(prefab) {
            object = prefab->actor()->clone(parent);
        }
    } else {
        object = Engine::composeActor(type, m_controller->findFreeObjectName(m_type, parent), parent);
    }

    return object;
}

void CreateObject::resolveUUID(Object::ObjectList &list, Object *root, bool generate) {
    if(generate) {
        for(auto it : list) {
            uint32_t origin = it->clonedFrom();
            if(origin == 0) {
                origin = Mathf::hashString(ObjectController::pathTo(it, root));
            }

            m_staticIds[origin] = it->uuid();
        }
    } else {
        for(auto it : list) {
            uint32_t origin = it->clonedFrom();
            if(origin == 0) {
                origin = Mathf::hashString(ObjectController::pathTo(it, root));
            }

            auto staticIt = m_staticIds.find(origin);
            if(staticIt != m_staticIds.end()) {
                Engine::replaceUUID(it, staticIt->second);
            }
        }
    }
}
