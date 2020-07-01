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

    static bool isPointer(const char *name) {
        for(uint32_t i = 0; i < strlen(name); i++) {
            if(name[i] == '*') {
                return true;
            }
        }
        return false;
    }

    uint8_t m_Layers;

    bool m_Enable;

    Transform *m_pTransform;

    Actor *m_pPrefab;

    Scene *m_pScene;
};
/*!
    \class Actor
    \brief Base class for all entities in Thunder Engine.
    \inmodule Engine

    The Actor probably is the most important class in the Thunder Engine.
    It represents all objects on the scene like 3D meshes, light sources, effects and many more.
    You should think about Actor as a key chain for the various Components.
    You can add and remove any components you like at any time except the Transform component.
    The Transform component must persist constantly and you shoudn't remove it.
*/

Actor::Actor() :
        p_ptr(new ActorPrivate) {

}

Actor::~Actor() {
    delete p_ptr;
}
/*!
    Returns true in case of Actor is enabled; otherwise returns false.
    Disabled Actors becomes invisible for the user.
    By default the property is \b true.
*/
bool Actor::isEnabled() const {
    PROFILE_FUNCTION();
    return p_ptr->m_Enable;
}
/*!
    Marks this Actor as enabled or disabled.
    Disabled Actors becomes invisible for the user.
*/
void Actor::setEnabled(const bool enabled) {
    PROFILE_FUNCTION();
    p_ptr->m_Enable = enabled;
}
/*!
    Returns the layers list for the this Actor as a bit mask.
    The layers used for the various purposes like filtering objects before rendering.
*/
uint8_t Actor::layers() const {
    PROFILE_FUNCTION();
    return p_ptr->m_Layers;
}
/*!
    Returns the Transform component attached to this Actor.
    If no Transform component found this method will create a new one.
*/
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
/*!
    Returns the scene where actor attached to.
*/
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
/*!
    Returns the component with \a type if one is attached to this Actor; otherwise returns nullptr.
*/
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
/*!
    Returns the component with \a type in the Actor's children using depth search.
    A component is returned only if it's found on a current Actor; otherwise returns nullptr.
*/
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
/*!
    Assigns the list of \a layers for this Actor as a bitmask.
*/
void Actor::setLayers(const uint8_t layers) {
    PROFILE_FUNCTION();
    p_ptr->m_Layers = layers;
}
/*!
    Returns created component with specified \a type;
*/
Component *Actor::addComponent(const string type) {
    PROFILE_FUNCTION();
    return static_cast<Component *>(Engine::objectCreate(type, type, this));
}
/*!
    \internal
*/
bool Actor::isSerializable() const {
    PROFILE_FUNCTION();

    bool result = (clonedFrom() == 0 || isPrefab());
    /* To be used in future
    if(!result) {
        for(auto it : getChildren()) {
            if(it->clonedFrom() == 0) {
                return true;
            }
        }
    }
    */
    return result;
}
/*!
    Makes the actor a child of the \a parent.
*/
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
/*!
    Returns true in case the current object is an instance of the serialized prefab structure; otherwise returns false.
*/
bool Actor::isPrefab() const {
    PROFILE_FUNCTION();
    return (p_ptr->m_pPrefab != nullptr);
}
/*!
    Marks this Actor as an instance of the \a prefab structure.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
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
/*!
    \internal
*/
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

        it = data.find(DATA);
        if(it != data.end()) {
            List objects;
            enumObjects(this, objects);

            typedef unordered_map<uint32_t, Object *> ObjectMap;
            ObjectMap cache;
            for(auto &object : objects) {
                cache[object->clonedFrom()] = object;
            }

            const VariantList &list = (*it).second.toList();
            for(auto &item : list) {
                const VariantList &fields = item.toList();

                uint32_t cloned = static_cast<uint32_t>(fields.front().toInt());
                auto object = cache.find(cloned);
                if(object != cache.end()) {
                    const MetaObject *meta = (*object).second->metaObject();
                    for(auto &property : fields.back().toMap()) {
                        int32_t index = meta->indexOfProperty(property.first.c_str());
                        if(index > -1) {
                            MetaProperty prop = meta->property(index);
                            Variant var = property.second;
                            if(prop.type().flags() & MetaType::BASE_OBJECT) {
                                if(var.type() == MetaType::STRING) { // Asset
                                    Object *res = Engine::loadResource(var.toString());
                                    if(res) {
                                        var = Variant(prop.read((*object).second).userType(), &res);
                                    }
                                } else if(var.type() == MetaType::VARIANTLIST) { // Component
                                    VariantList array = var.toList();
                                    uint32_t uuid = static_cast<uint32_t>(array.front().toInt());
                                    if(uuid == 0) {
                                        uuid = static_cast<uint32_t>(array.back().toInt());
                                    }
                                    Object *obj = Engine::findObject(uuid, Engine::findRoot(this));
                                    if(obj) {
                                        var = Variant(prop.read((*object).second).userType(), &obj);
                                    }
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
/*!
    \internal
*/
VariantMap Actor::saveUserData() const {
    PROFILE_FUNCTION();
    VariantMap result = Object::saveUserData();
    if(p_ptr->m_pPrefab) {
        string ref = Engine::reference(p_ptr->m_pPrefab);
        if(!ref.empty()) {
            result[PREFAB] = ref;

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
                            Variant lv = lp.read((*fab).second);
                            Variant rv = rp.read(it);
                            if(lv != rv) {
                                 if(lp.type().flags() & MetaType::BASE_OBJECT) {
                                    Object *lo = *(reinterpret_cast<Object **>(lv.data()));
                                    Object *ro = *(reinterpret_cast<Object **>(rv.data()));

                                    string lref = Engine::reference(lo);
                                    string rref = Engine::reference(ro);
                                    if(lref != rref) {
                                        prop[rp.name()] = rref;
                                    }

                                    if(rref.empty() && lref.empty()) {
                                        if((lo == nullptr && ro) || (ro && lo->uuid() != ro->uuid())) {
                                            VariantList array;
                                            array.push_back(static_cast<int32_t>(ro->clonedFrom()));
                                            array.push_back(static_cast<int32_t>(ro->uuid()));
                                            prop[rp.name()] = array;
                                        }
                                    }
                                } else {
                                    prop[rp.name()] = rv;
                                }
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
