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

                auto position = m_positions.find(child->uuid());
                Object *parent = child->parent();
                if(position != m_positions.end() && parent) {
                    child->setParent(parent, position->second);
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
    m_positions.clear();

    std::list<uint32_t> list;
    bool isComponent = false;

    for(auto it : m_objects)  {
        Object *object = Engine::findObject(it);
        if(object) {
            m_dump.push_back(Engine::toVariant(object, true));

            Object::ObjectList children;
            Engine::enumObjects(object, children);
            for(auto child : children) {
                if(child->clonedFrom() != 0) {
                    m_cloneCache[child->uuid()] = child->clonedFrom();
                }

                Object *parent = child->parent();
                if(parent) {
                    int32_t position = 0;
                    for(auto sibling : parent->getChildren()) {
                        if(sibling == child) {
                            m_positions[child->uuid()] = position;
                            break;
                        }
                        position++;
                    }
                }
            }

            list.push_back(object->parent()->uuid());

            Actor *actor = dynamic_cast<Actor *>(object);
            if(actor) {
                scenes.insert(actor->scene());
            } else {
                Component *component = dynamic_cast<Component *>(object);
                if(component) {
                    isComponent = true;
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
                    Object *parent = child->parent();
                    if(parent) {
                        int32_t position = 0;
                        for(auto sibling : parent->getChildren()) {
                            if(sibling == child) {
                                m_positions[child->uuid()] = position;
                                break;
                            }
                            position++;
                        }
                    }

                    if(prefab->contains(child->clonedFrom())) {
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

    if(!isComponent) {
        m_controller->clear();
        m_controller->selectActors(list);
    }

    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            delete object;
        }
    }

    if(isComponent) {
        m_controller->selectActors(list);
    }

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
