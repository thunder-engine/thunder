#include "deleteobjects.h"

#include <components/world.h>
#include <components/actor.h>
#include <components/component.h>

#include <set>

DeleteObjects::DeleteObjects(const Object::ObjectList &objects, ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_controller(ctrl),
        m_prefab(0) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }

    Prefab *fab = m_controller->isolatedPrefab();
    if(fab) {
        m_prefab = fab->uuid();
    }
}

void DeleteObjects::undo() {
    std::set<Scene *> scenes;

    std::list<uint32_t> list;

    for(auto &ref : m_dump) {
        Object *object = Engine::toObject(ref);
        if(object) {
            Object::ObjectList children;
            Engine::enumObjects(object, children);

            for(auto child : children) {
                auto origin = m_cloneCache.find(child->uuid());
                if(origin != m_cloneCache.end()) {
                    Engine::replaceClonedUUID(child, origin->second);
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
                    list.push_back(component->actor()->uuid());
                }
            }
        }
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

void DeleteObjects::redo() {
    std::set<Scene *> scenes;

    m_dump.clear();
    m_cloneCache.clear();

    std::list<uint32_t> list;

    for(auto it : m_objects)  {
        Object *object = Engine::findObject(it);
        if(object) {
            m_dump.push_back(Engine::toVariant(object, true));

            list.push_back(object->parent()->uuid());

            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            } else {
                Component *component = dynamic_cast<Component *>(object);
                if(component) {
                    scenes.insert(component->scene());
                }
            }
        }
    }

    if(m_prefab) {
        Prefab *prefab = dynamic_cast<Prefab *>(Engine::findObject(m_prefab));

        for(auto it : m_objects)  {
            Object::ObjectList clones;
            m_controller->getClones(clones, it, Engine::world());

            for(auto clone : clones) {
                Object *cloneRoot = getRoot(clone, prefab->actor()->uuid());

                Object::ObjectList children;
                Engine::enumObjects(clone, children);

                for(auto child : children) {
                    if(prefab->contains(child->uuid())) {
                        m_cloneCache[child->uuid()] = child->clonedFrom();
                    } else if(cloneRoot && dynamic_cast<Actor *>(child) != nullptr) {
                        // This child Actor isn't belongs to current prefab. Moving to instance root.
                        child->setParent(cloneRoot);
                    }
                }

                m_dump.push_back(Engine::toVariant(clone, true));

                delete clone;
            }
        }
    }

    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            delete object;
        }
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

Object *DeleteObjects::getRoot(Object *object, uint32_t originRoot) const {
    if(object->clonedFrom() == originRoot) {
        return object;
    } else {
        Object *parent = object->parent();
        if(parent) {
            return getRoot(parent, originRoot);
        }
    }
    return nullptr;
}
