#include "components/actor.h"
#include "components/scene.h"
#include "components/transform.h"

#include "commandbuffer.h"

#include <cstring>

#define PREFAB  "Prefab"
#define DATA    "PrefabData"

class ActorPrivate {
public:
    ActorPrivate() :
        m_Layers(ICommandBuffer::DEFAULT | ICommandBuffer::RAYCAST | ICommandBuffer::SHADOWCAST| ICommandBuffer::TRANSLUCENT),
        m_Enable(true),
        m_pTransform(nullptr),
        m_pPrefab(nullptr),
        m_pScene(nullptr) {

    }

    uint8_t m_Layers;

    bool m_Enable;

    Transform *m_pTransform;

    Actor *m_pPrefab;

    Scene *m_pScene;
};

Actor::Actor() :
        p_ptr(new ActorPrivate) {

}

Actor::~Actor() {
    delete p_ptr;
}

bool Actor::isEnabled() const {
    PROFILE_FUNCTION();
    return p_ptr->m_Enable;
}

void Actor::setEnabled(const bool enabled) {
    PROFILE_FUNCTION();
    p_ptr->m_Enable = enabled;
}

uint8_t Actor::layers() const {
    PROFILE_FUNCTION();
    return p_ptr->m_Layers;
}

Transform *Actor::transform() {
    PROFILE_FUNCTION();
    if(p_ptr->m_pTransform == nullptr) {
        p_ptr->m_pTransform = static_cast<Transform *>(component("Transform"));
        if(p_ptr->m_pTransform == nullptr) {
            p_ptr->m_pTransform = static_cast<Transform *>(addComponent("Transform"));
        }
        Actor *p = dynamic_cast<Actor *>(parent());
        if(p) {
            p_ptr->m_pTransform->setParentTransform(p->transform(), true);
        }
    }
    return p_ptr->m_pTransform;
}

Scene *Actor::scene() {
    PROFILE_FUNCTION();
    if(p_ptr->m_pScene == nullptr) {
        Object *scene = parent();
        if(scene) {
            while(scene->parent() != nullptr) {
                scene = scene->parent();
            }
            p_ptr->m_pScene = dynamic_cast<Scene *>(scene);
        }
    }
    return p_ptr->m_pScene;
}

Component *Actor::component(const string type) {
    PROFILE_FUNCTION();
    for(auto it : getChildren()) {
        const MetaObject *meta = it->metaObject();
        if(meta->canCastTo(type.c_str())) {
            return static_cast<Component *>(it);
        }
    }
    return nullptr;
}

Component *componentInChildHelper(const string &type, Object *parent) {
    PROFILE_FUNCTION();
    for(auto it : parent->getChildren()) {
        const MetaObject *meta = it->metaObject();
        if(meta->canCastTo(type.c_str())) {
            return static_cast<Component *>(it);
        } else {
            Component *result = componentInChildHelper(type, it);
            if(result) {
                return static_cast<Component *>(result);
            }
        }
    }
    return nullptr;
}

Component *Actor::componentInChild(const string type) {
    PROFILE_FUNCTION();
    for(auto it : getChildren()) {
        Component *result = componentInChildHelper(type, it);
        if(result) {
            return static_cast<Component *>(result);
        }
    }
    return nullptr;
}

void Actor::setLayers(const uint8_t layers) {
    PROFILE_FUNCTION();
    p_ptr->m_Layers = layers;
}

Component *Actor::addComponent(const string type) {
    PROFILE_FUNCTION();
    return static_cast<Component *>(Engine::objectCreate(type, type, this));
}

bool Actor::isSerializable() const {
    PROFILE_FUNCTION();
    Actor *actor   = dynamic_cast<Actor *>(parent());
    if(actor) {
        return !actor->isPrefab();
    }
    return true;
}

void Actor::setParent(Object *parent) {
    PROFILE_FUNCTION();
    if(p_ptr->m_pTransform) {
        Object::setParent(parent);

        Actor *actor = dynamic_cast<Actor *>(parent);
        if(actor) {
            p_ptr->m_pTransform->setParentTransform(actor->transform());
        }
    } else {
        Object::setParent(parent);
    }
}

bool Actor::isPrefab() const {
    PROFILE_FUNCTION();
    return (p_ptr->m_pPrefab != nullptr);
}

void Actor::setPrefab(Actor *prefab) {
    PROFILE_FUNCTION();
    p_ptr->m_pPrefab = prefab;
}

typedef list<Object *> List;
void enumObjects(Object *object, List &list) {
    PROFILE_FUNCTION();
    list.push_back(object);
    for(const auto &it : object->getChildren()) {
        enumObjects(it, list);
    }
}

void Actor::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();
    auto it = data.find(PREFAB);
    if(it != data.end()) {
        setPrefab(Engine::loadResource<Actor>((*it).second.toString()));
        if(p_ptr->m_pPrefab) {
            Actor *actor = static_cast<Actor *>(p_ptr->m_pPrefab->clone());

            Object::ObjectList list = actor->getChildren();
            for(auto &it : list) {
                it->setParent(this);
            }
            delete actor;
        }

        it  = data.find(DATA);
        if(it != data.end()) {
            List objects;
            enumObjects(this, objects);

            typedef unordered_map<uint32_t, Object *> ObjectMap;
            ObjectMap cache;
            for(auto &it : objects) {
                cache[it->clonedFrom()] = it;
            }

            const VariantList &list = (*it).second.toList();
            for(auto &it : list) {
                const VariantList &fields   = it.toList();

                uint32_t cloned = static_cast<uint32_t>(fields.front().toInt());
                auto object = cache.find(cloned);
                if(object != cache.end()) {
                    const MetaObject *meta = (*object).second->metaObject();
                    for(auto &property : fields.back().toMap()) {
                        int32_t index = meta->indexOfProperty(property.first.c_str());
                        if(index > -1) {
                            MetaProperty prop = meta->property(index);
                            bool ptr = false;

                            const char *name = prop.type().name();
                            for(uint32_t i = 0; i < strlen(name); i++) {
                                if(name[i] == '*') {
                                    ptr = true;
                                    break;
                                }
                            }
                            Variant var = property.second;
                            if(ptr && var.type() == MetaType::STRING) {
                                Object *res = Engine::loadResource(var.toString());
                                if(res) {
                                    var = Variant(prop.read((*object).second).userType(), &res);
                                }
                            }
                            prop.write((*object).second, var);
                        }
                    }
                }
            }
        }
    }
}

typedef list<const Object *> ConstList;
void enumConstObjects(const Object *object, ConstList &list) {
    PROFILE_FUNCTION();
    list.push_back(object);
    for(const auto &it : object->getChildren()) {
        enumConstObjects(it, list);
    }
}

VariantMap Actor::saveUserData() const {
    PROFILE_FUNCTION();
    VariantMap result   = Object::saveUserData();
    if(p_ptr->m_pPrefab) {
        string ref      = Engine::reference(p_ptr->m_pPrefab);
        if(!ref.empty()) {
            result[PREFAB]  = ref;

            ConstList prefabs;
            enumConstObjects(p_ptr->m_pPrefab, prefabs);

            typedef unordered_map<uint32_t, const Object *> ObjectMap;
            ObjectMap cache;
            for(auto it : prefabs) {
                cache[it->uuid()] = it;
            }
            VariantList list;

            ConstList objects;
            enumConstObjects(this, objects);
            for(auto it : objects) {
                uint32_t cloned = it->clonedFrom();
                if(cloned) {
                    auto fab = cache.find(cloned);
                    if(fab != cache.end()) {
                        VariantMap prop;

                        const MetaObject *meta = it->metaObject();
                        int count  = meta->propertyCount();
                        for(int i = 0; i < count; i++) {
                            MetaProperty lp = (*fab).second->metaObject()->property(i);
                            MetaProperty rp = meta->property(i);
                            Variant lv  = lp.read((*fab).second);
                            Variant rv  = rp.read(it);
                            if(lv != rv) {
                                const char *name = rp.type().name();
                                for(uint32_t i = 0; i < strlen(name); i++) {
                                    if(name[i] == '*') {
                                        Object *o = *(reinterpret_cast<Object **>(rv.data()));
                                        string ref = Engine::reference(o);
                                        if(!ref.empty()) {
                                            rv = ref;
                                        }
                                        break;
                                    }
                                }

                                prop[rp.name()] = rv;
                            }
                        }

                        if(!prop.empty()) {
                            VariantList array;
                            array.push_back(static_cast<int32_t>(cloned));
                            array.push_back(prop);
                            list.push_back(array);
                        }
                    }
                }
            }

            if(!list.empty()) {
                result[DATA] = list;
            }
        }
    }
    return result;
}
