#include "components/actor.h"

#include "components/transform.h"

#include "commandbuffer.h"

#define PREFAB  "Prefab"
#define DATA    "PrefabData"

Actor::Actor() :
        m_Layers(ICommandBuffer::DEFAULT | ICommandBuffer::RAYCAST | ICommandBuffer::SHADOWCAST| ICommandBuffer::TRANSLUCENT),
        m_Enable(true),
        m_pTransform(nullptr),
        m_pPrefab(nullptr) {

}

bool Actor::isEnable() const {
    return m_Enable;
}

void Actor::setEnable(const bool enable) {
    m_Enable    = enable;
}

uint8_t Actor::layers() const {
    return m_Layers;
}

Transform *Actor::transform() {
    if(m_pTransform == nullptr) {
        m_pTransform    = component<Transform>();
        if(m_pTransform == nullptr) {
            m_pTransform    = addComponent<Transform>();
        }
    }
    return m_pTransform;
}

Component *Actor::findComponent(const char *type) {
    for(auto it : getChildren()) {
        const MetaObject *meta = it->metaObject();
        if(meta->canCastTo(type)) {
            return static_cast<Component *>(it);
        }
    }
    return nullptr;
}

void Actor::setLayers(const uint8_t layers) {
    m_Layers    = layers;
}

Component *Actor::createComponent(const string type) {
    return static_cast<Component *>(Engine::objectCreateImpl(type, type, this));
}

void Actor::addChild(Object *value) {
    Object::addChild(value);

    Transform *t    = dynamic_cast<Transform *>(value);
    if(t) {
        if(m_pTransform != nullptr) {
            delete m_pTransform;
        }
        m_pTransform    = t;
    }
}

bool Actor::isSerializable() const {
    Actor *actor   = dynamic_cast<Actor *>(parent());
    if(actor) {
        return !actor->isPrefab();
    }
    return true;
}

void Actor::setParent(Object *parent) {
    Transform *t    = transform();
    if(t) {
        Vector3 p   = t->worldPosition();
        Vector3 e   = t->worldEuler();
        Vector3 s   = t->worldScale();

        Object::setParent(parent);

        Actor *actor   = dynamic_cast<Actor *>(parent);
        if(actor) {
            Transform *par  = actor->transform();
            if(par) {
                Vector3 scale   = par->worldScale();
                scale   = Vector3(1.0 / scale.x, 1.0 / scale.y, 1.0 / scale.z);

                p   = par->worldRotation().inverse() * ((p - par->worldPosition()) * scale);
                e   = e - par->worldEuler();
                s   = s * scale;
            }
        }

        t->setPosition(p);
        t->setEuler(e);
        t->setScale(s);
    } else {
        Object::setParent(parent);
    }
}

bool Actor::isPrefab() const {
    return (m_pPrefab != nullptr);
}

void Actor::setPrefab(Actor *prefab) {
    m_pPrefab   = prefab;
}

typedef list<Object *> List;
void enumObjects(Object *object, List &list) {
    PROFILE_FUNCTION()
    list.push_back(object);
    for(const auto &it : object->getChildren()) {
        enumObjects(it, list);
    }
}

void Actor::loadUserData(const VariantMap &data) {
    auto it = data.find(PREFAB);
    if(it != data.end()) {
        setPrefab(Engine::loadResource<Actor>((*it).second.toString()));
        Actor *actor    = static_cast<Actor *>(m_pPrefab->clone());

        Object::ObjectList list = actor->getChildren();
        for(auto &it : list) {
            it->setParent(this);
        }
        delete actor;

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

                uint32_t cloned = fields.front().toInt();
                auto object = cache.find(cloned);
                if(object != cache.end()) {
                    const MetaObject *meta = (*object).second->metaObject();
                    for(auto &property : fields.back().toMap()) {
                        int32_t index   = meta->indexOfProperty(property.first.c_str());
                        if(index > -1) {
                             meta->property(index).write((*object).second, property.second);
                        }

                    }
                }
            }
        }
    }
}

typedef list<const Object *> ConstList;
void enumConstObjects(const Object *object, ConstList &list) {
    PROFILE_FUNCTION()
    list.push_back(object);
    for(const auto &it : object->getChildren()) {
        enumConstObjects(it, list);
    }
}

VariantMap Actor::saveUserData() const {
    VariantMap result   = Object::saveUserData();
    if(m_pPrefab) {
        string ref      = Engine::reference(m_pPrefab);
        if(!ref.empty()) {
            result[PREFAB]  = ref;

            ConstList prefabs;
            enumConstObjects(m_pPrefab, prefabs);

            typedef unordered_map<uint32_t, const Object *> ObjectMap;
            ObjectMap cache;
            for(auto it : prefabs) {
                cache[it->uuid()] = it;
            }
            VariantList list;

            ConstList objects;
            enumConstObjects(this, objects);
            for(auto it : objects) {
                int32_t cloned  = it->clonedFrom();
                if(cloned) {
                    auto fab = cache.find(cloned);
                    if(fab != cache.end()) {
                        VariantMap map;

                        const MetaObject *meta = it->metaObject();
                        int count  = meta->propertyCount();
                        for(int i = 0; i < count; i++) {
                            MetaProperty lp = (*fab).second->metaObject()->property(i);
                            MetaProperty rp = meta->property(i);
                            Variant lv  = lp.read((*fab).second);
                            Variant rv  = rp.read(it);
                            if(lv != rv) {
                                map[rp.name()]  = rv;
                            }
                        }

                        if(!map.empty()) {
                            VariantList array;
                            array.push_back(cloned);
                            array.push_back(map);
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
