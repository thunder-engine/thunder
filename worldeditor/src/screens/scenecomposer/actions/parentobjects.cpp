#include "parentobjects.h"

#include <components/world.h>
#include <components/actor.h>
#include <components/component.h>

#include <set>

ParentObjects::ParentObjects(const Object::ObjectList &objects, Object *parent, int32_t position, ObjectController *ctrl, const TString &name, UndoCommand *group) :
        UndoCommand(name, group),
        m_controller(ctrl),
        m_prefab(0),
        m_parent(parent->uuid()),
        m_position(position) {

    for(auto it : objects) {
        m_objects.push_back(it->uuid());
    }

    Prefab *fab = m_controller->isolatedPrefab();
    if(fab) {
        m_prefab = fab->uuid();
    }
}

void ParentObjects::undo() {
    std::set<Scene *> scenes;

    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            auto cacheIt = m_parentCache.find(it);
            if(cacheIt != m_parentCache.end()) {
                Object *parent = Engine::findObject(cacheIt->second);
                if(parent) {
                    object->setParent(parent);

                    Actor *actor = dynamic_cast<Actor *>(parent);
                    if(actor) {
                        scenes.insert(actor->scene());
                    } else {
                        Component *component = dynamic_cast<Component *>(parent);
                        if(component) {
                            scenes.insert(component->scene());
                        }
                    }
                }

                if(m_prefab) {
                    Object::ObjectList parentList;
                    m_controller->getClones(parentList, cacheIt->second, Engine::world());

                    Object::ObjectList objectList;
                    m_controller->getClones(objectList, it, Engine::world());

                    assert(parentList.size() == objectList.size());
                    auto parentIt = parentList.begin();
                    for(auto clone : objectList) {
                        clone->setParent(*parentIt, m_position);

                        ++parentIt;
                    }
                }
            }
        }
    }

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }

    if(m_prefab) {
        emit m_controller->sceneUpdated(Engine::findObject(m_prefab));
    }
}

void ParentObjects::redo() {
    std::set<Scene *> scenes;

    m_parentCache.clear();

    Object::ObjectList parentList;
    if(m_prefab) {
        m_controller->getClones(parentList, m_parent, Engine::world());
    }

    for(auto it : m_objects) {
        Object *object = Engine::findObject(it);
        if(object) {
            m_parentCache[it] = object->parent()->uuid();

            Object *parent = Engine::findObject(m_parent);
            if(parent) {
                object->setParent(parent, m_position);

                Actor *actor = dynamic_cast<Actor *>(parent);
                if(actor) {
                    scenes.insert(actor->scene());
                } else {
                    Component *component = dynamic_cast<Component *>(parent);
                    if(component) {
                        scenes.insert(component->scene());
                    }
                }
            }

            if(m_prefab) {
                Object::ObjectList objectList;
                m_controller->getClones(objectList, it, Engine::world());

                assert(parentList.size() == objectList.size());
                auto parentIt = parentList.begin();
                for(auto clone : objectList) {
                    clone->setParent(*parentIt, m_position);

                    ++parentIt;
                }
            }
        }
    }

    for(auto it : scenes) {
        emit m_controller->sceneUpdated(it);
    }

    if(m_prefab) {
        emit m_controller->sceneUpdated(Engine::findObject(m_prefab));
    }
}
