#include "components/actor.h"
#include "components/scene.h"
#include "components/transform.h"

#include "resources/prefab.h"

#include "systems/resourcesystem.h"

#include "commandbuffer.h"

#include <cstring>

#include <json.h>
#include <log.h>

const char *FLAGS   ("Flags");
const char *PREFAB  ("Prefab");
const char *DATA    ("PrefabData");
const char *STATIC  ("Static");
const char *DELETED ("Deleted");
const char *TRANSFORM("Transform");

class ActorPrivate : public Resource::IObserver {
public:
    explicit ActorPrivate(Actor *actor) :
        m_transform(nullptr),
        m_prefab(nullptr),
        m_scene(nullptr),
        m_actor(actor),
        m_layers(CommandBuffer::DEFAULT | CommandBuffer::RAYCAST | CommandBuffer::SHADOWCAST | CommandBuffer::TRANSLUCENT),
        m_flags(Actor::ENABLE | Actor::SELECTABLE),
        m_hierarchyEnable(m_flags & Actor::ENABLE),
        m_static(false) {

    }

    ~ActorPrivate() {
        if(m_prefab) {
            m_prefab->unsubscribe(this);
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) {
        if(resource == m_prefab) {
            switch(state) {
                case Resource::Loading: {
                    m_data = m_actor->saveUserData();
                } break;
                case Resource::Ready: {
                    ActorPrivate::List prefabObjects;
                    ActorPrivate::enumObjects(m_prefab->actor(), prefabObjects);

                    ActorPrivate::List deleteObjects;
                    ActorPrivate::enumObjects(m_actor, deleteObjects);

                    list<pair<Object *, Object *>> array;

                    for(auto prefabObject : prefabObjects) {
                        bool create = true;
                        auto it = deleteObjects.begin();
                        while(it != deleteObjects.end()) {
                            Object *clone = *it;
                            if(prefabObject->uuid() == clone->clonedFrom()) {
                                array.push_back(make_pair(prefabObject, clone));
                                it = deleteObjects.erase(it);
                                create = false;
                                break;
                            } else if(clone->clonedFrom() == 0) { // probably was created right in instance we don't need to sync it
                                it = deleteObjects.erase(it);
                                create = false;
                                break;
                            }
                            ++it;
                        }
                        if(create) {
                            Object *parent = System::findObject(prefabObject->parent()->uuid(), m_actor);
                            Object *result = prefabObject->clone(parent ? parent : m_actor);

                            array.push_back(make_pair(prefabObject, result));
                        }
                    }

                    for(auto it : array) {
                        const MetaObject *meta = it.first->metaObject();
                        for(int i = 0; i < meta->propertyCount(); i++) {
                            MetaProperty origin = meta->property(i);
                            MetaProperty target = it.second->metaObject()->property(i);
                            if(origin.isValid() && target.isValid()) {
                                Variant data = origin.read(it.first);
                                if(origin.type().flags() & MetaType::BASE_OBJECT) {
                                    Object *ro = *(reinterpret_cast<Object **>(data.data()));

                                    for(auto &item : array) {
                                        if(item.first == ro) {
                                            ro = item.second;
                                            break;
                                        }
                                    }

                                    data = Variant(data.userType(), &ro);
                                }
                                target.write(it.second, data);
                            }
                        }

                        for(auto item : it.first->getReceivers()) {
                            MetaMethod signal = it.second->metaObject()->method(item.signal);
                            MetaMethod method = item.receiver->metaObject()->method(item.method);
                            Object::connect(it.second, (to_string(1) + signal.signature()).c_str(),
                                            item.receiver, (to_string((method.type() == MetaMethod::Signal) ? 1 : 2) + method.signature()).c_str());
                        }
                    }

                    for(auto it : deleteObjects) {
                        delete it;
                    }

                    m_actor->loadUserData(m_data);
                } break;
                case Resource::ToBeDeleted: {
                    /// \todo Unload prefab related components
                    for(auto &it : m_actor->getChildren()) {
                        if(it != m_actor->transform()) {
                            it->deleteLater();
                        }
                    }
                    m_data = m_actor->saveUserData();
                    m_prefab = nullptr;
                } break;
                default: break;
            }
        }
    }

    typedef list<Object *> List;
    static void enumObjects(Object *object, List &list) {
        PROFILE_FUNCTION();
        list.push_back(object);
        for(const auto &it : object->getChildren()) {
            enumObjects(it, list);
        }
    }

    typedef list<const Object *> ConstList;
    static void enumConstObjects(const Object *object, ConstList &list) {
        PROFILE_FUNCTION();
        list.push_back(object);
        for(const auto &it : object->getChildren()) {
            enumConstObjects(it, list);
        }
    }

    static Component *componentInChildHelper(const string &type, Object *parent) {
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

    VariantMap m_data;

    Transform *m_transform;

    Prefab *m_prefab;

    Scene *m_scene;

    Actor *m_actor;

    int32_t m_layers;

    int m_flags;

    bool m_hierarchyEnable;

    bool m_static;
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

/*!
    \enum Actor::HideFlags

    \value ENABLE \c This Actor can be visible on the screen and can be updated in the game cycle.
    \value SELECTABLE \c This Actor can be selected in the Editor.
*/

Actor::Actor() :
        p_ptr(new ActorPrivate(this)) {

}

Actor::~Actor() {
    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    Returns true in case of Actor is enabled; otherwise returns false.
    Disabled Actors becomes invisible for the user.
    By default the property is \b true.
*/
bool Actor::isEnabled() const {
    PROFILE_FUNCTION();
    return p_ptr->m_flags & ENABLE;
}
/*!
    Marks this Actor as \a enabled or disabled.
    Disabled Actors becomes invisible for the user.
*/
void Actor::setEnabled(const bool enabled) {
    PROFILE_FUNCTION();
    if(enabled) {
        p_ptr->m_flags |= ENABLE;
    } else {
        p_ptr->m_flags &= ~ENABLE;
    }

    setHierarchyEnabled(enabled);
}
/*!
    Returns a set of Actor::HideFlags applied to this Actor.
*/
int Actor::hideFlags() const {
    PROFILE_FUNCTION();

    return p_ptr->m_flags;
}
/*!
    Applies a new set of Actor::HideFlags flags to this Actor.
*/
void Actor::setHideFlags(int flags) {
    PROFILE_FUNCTION();

    bool old = isEnabled();

    p_ptr->m_flags = flags;

    bool current = isEnabled();
    if(old != current) {
        setHierarchyEnabled(current);
    }
}
/*!
    Returns false in case of one of Actors in top hierarchy was disabled; otherwise returns true.
*/
bool Actor::isEnabledInHierarchy() const {
    return (p_ptr->m_hierarchyEnable && isEnabled());
}
/*!
    \internal
*/
void Actor::setHierarchyEnabled(bool enabled) {
    p_ptr->m_hierarchyEnable = enabled;
    for(auto it : getChildren()) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor && actor->isEnabled()) {
            actor->setHierarchyEnabled(enabled);
        }
    }
}
/*!
    Returns true if this actor will not be moved during the game; otherwise returns false.
*/
bool Actor::isStatic() const {
    return p_ptr->m_static;
}
/*!
    Marks current Actor as static or dynamic (by default).
    This \a flag can help to optimize rendering.
*/
void Actor::setStatic(const bool flag) {
    p_ptr->m_static = flag;
}
/*!
    Returns the layers list for the this Actor as a bit mask.
    The layers used for the various purposes like filtering objects before rendering.
*/
int Actor::layers() const {
    PROFILE_FUNCTION();
    return p_ptr->m_layers;
}
/*!
    Assigns the list of \a layers for this Actor as a bitmask.
*/
void Actor::setLayers(const int layers) {
    PROFILE_FUNCTION();
    p_ptr->m_layers = layers;
}
/*!
    Returns the Transform component attached to this Actor.
*/
Transform *Actor::transform() {
    PROFILE_FUNCTION();
    if(p_ptr->m_transform == nullptr) {
        p_ptr->m_transform = static_cast<Transform *>(component(TRANSFORM));
    }
    return p_ptr->m_transform;
}
/*!
    Returns the scene where actor attached to.
*/
Scene *Actor::scene() {
    PROFILE_FUNCTION();
    return p_ptr->m_scene;
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
/*!
    Returns the component with \a type in the Actor's children using depth search.
    A component is returned only if it's found on a current Actor; otherwise returns nullptr.
*/
Component *Actor::componentInChild(const string type) {
    PROFILE_FUNCTION();
    for(auto it : getChildren()) {
        Component *result = ActorPrivate::componentInChildHelper(type, it);
        if(result) {
            return static_cast<Component *>(result);
        }
    }
    return nullptr;
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

    bool result = (clonedFrom() == 0 || isInstance());

    return result;
}
/*!
    \internal
*/
Object *Actor::clone(Object *parent) {
    PROFILE_FUNCTION();
    Actor *result = static_cast<Actor *>(Object::clone(parent));
    Prefab *prefab = dynamic_cast<Prefab *>(Object::parent());
    if(prefab) {
        result->setPrefab(prefab);
    } else {
        result->setPrefab(p_ptr->m_prefab);
    }
    return result;
}
/*!
    \internal
*/
void Actor::clearCloneRef() {
    PROFILE_FUNCTION();
    if(p_ptr->m_prefab == nullptr) {
        Object::clearCloneRef();

        for(auto it : getChildren()) {
            it->clearCloneRef();
        }
    }
}
/*!
    Makes the actor a child of the \a parent at given \a position.
    \note Please ignore the \a force flag it will be provided by the default.
*/
void Actor::setParent(Object *parent, int32_t position, bool force) {
    PROFILE_FUNCTION();
    if(parent == this || (Object::parent() == parent && position == -1)) {
        return;
    }

    p_ptr->m_scene = nullptr;

    Actor *actor = dynamic_cast<Actor *>(parent);
    if(actor) {
        p_ptr->m_scene = actor->scene();
        p_ptr->m_hierarchyEnable = actor->p_ptr->m_hierarchyEnable;
    } else {
        Scene *scene = dynamic_cast<Scene *>(parent);
        if(scene) {
            p_ptr->m_scene = scene;
        }
    }
    if(p_ptr->m_transform) {
        Object::setParent(parent, position, force);
        if(actor) {
            p_ptr->m_transform->setParentTransform(actor->transform(), force);
        }
    } else {
        Object::setParent(parent, position);
    }

    for(auto it : getChildren()) {
        Component *component = dynamic_cast<Component *>(it);
        if(component) {
            component->actorParentChanged();
        } else {
            Actor *child = dynamic_cast<Actor *>(it);
            if(child && actor) {
                child->p_ptr->m_scene = actor->p_ptr->m_scene;
            }
        }
    }
}
/*!
    Returns true in case the current object is an instance of the serialized prefab structure; otherwise returns false.
*/
bool Actor::isInstance() const {
    PROFILE_FUNCTION();
    return (p_ptr->m_prefab != nullptr);
}
/*!
    Return true if \a actor is a part of hiearhy.
*/
bool Actor::isInHierarchy(Actor *actor) const {
    if(this == actor) {
        return true;
    }
    Actor *p = static_cast<Actor *>(parent());
    if(p) {
        return p->isInHierarchy(actor);
    }

    return false;
}
/*!
    Returns a Prefab object from which the Actor was instanced.
    \internal
*/
Prefab *Actor::prefab() const {
    return p_ptr->m_prefab;
}
/*!
    Marks this Actor as an instance of the \a prefab structure.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
void Actor::setPrefab(Prefab *prefab) {
    PROFILE_FUNCTION();
    if(p_ptr->m_prefab) {
        p_ptr->m_prefab->unsubscribe(p_ptr);
    }
    p_ptr->m_prefab = prefab;
    if(p_ptr->m_prefab) {
        p_ptr->m_prefab->subscribe(p_ptr);
    } else {
        clearCloneRef();
    }
}
/*!
    \internal
*/
void Actor::loadObjectData(const VariantMap &data) {
    PROFILE_FUNCTION();

    auto it = data.find(PREFAB);
    if(it != data.end()) {
        setPrefab(dynamic_cast<Prefab *>(Engine::resourceSystem()->loadResource((*it).second.toString())));

        if(p_ptr->m_prefab) {
            Actor *actor = static_cast<Actor *>(p_ptr->m_prefab->actor()->clone());

            it = data.find(DELETED);
            if(it != data.end()) {
                for(auto &item : (*it).second.toList()) {
                    uint32_t uuid = static_cast<uint32_t>(item.toInt());
                    Object *result = ObjectSystem::findObject(uuid, actor);
                    if(result && result != actor) {
                        delete result;
                    }
                }
            }

            Object::ObjectList list = actor->getChildren();
            for(auto &it : list) {
                it->setParent(this);
            }
            delete actor;

            unordered_map<uint32_t, uint32_t> staticMap;
            auto it = data.find(STATIC);
            if(it != data.end()) {
                for(auto &item : (*it).second.toList()) {
                    VariantList array = item.toList();

                    uint32_t clone = static_cast<uint32_t>(array.front().toInt());
                    Object *result = ObjectSystem::findObject(clone, this);
                    if(result) {
                        ObjectSystem::replaceUUID(result, static_cast<uint32_t>(array.back().toInt()));
                    }
                }
            }
        } else {
            p_ptr->m_data = data;
        }
    }
}
/*!
    \internal
*/
void Actor::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    auto it = data.find(FLAGS);
    if(it != data.end()) {
        p_ptr->m_flags = it->second.toInt();
    }

    if(p_ptr->m_prefab) {
        it = data.find(DATA);
        if(it != data.end()) {
            ActorPrivate::List objects;
            ActorPrivate::enumObjects(this, objects);

            unordered_map<uint32_t, Object *> cacheMap;
            for(auto &object : objects) {
                uint32_t clone = object->clonedFrom();
                cacheMap[clone] = object;
            }

            const VariantList &list = (*it).second.toList();
            for(auto &item : list) {
                const VariantList &fields = item.toList();

                uint32_t cloned = static_cast<uint32_t>(fields.front().toInt());
                auto object = cacheMap.find(cloned);
                if(object != cacheMap.end()) {
                    const MetaObject *meta = (*object).second->metaObject();
                    for(auto &property : fields.back().toMap()) {
                        int32_t index = meta->indexOfProperty(property.first.c_str());
                        if(index > -1) {
                            MetaProperty prop = meta->property(index);
                            Variant var = property.second;
                            if(prop.type().flags() & MetaType::BASE_OBJECT) {
                                if(var.type() == MetaType::STRING) { // Asset
                                    Object *res = Engine::resourceSystem()->loadResource(var.toString());
                                    if(res) {
                                        var = Variant(prop.read((*object).second).userType(), &res);
                                    }
                                } else if(var.type() == MetaType::INTEGER) { // Component
                                    uint32_t uuid = static_cast<uint32_t>(var.toInt());

                                    Object *obj = Engine::findObject(uuid, this);
                                    if(obj == nullptr) {
                                        obj = Engine::findObject(uuid, Engine::findRoot(this));
                                    }
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
/*!
    \internal
*/
VariantMap Actor::saveUserData() const {
    PROFILE_FUNCTION();
    VariantMap result = Object::saveUserData();

    result[FLAGS] = p_ptr->m_flags;

    if(isInstance()) {
        string ref = Engine::reference(p_ptr->m_prefab);
        if(!ref.empty()) {
            result[PREFAB] = ref;

            ActorPrivate::ConstList prefabs;
            ActorPrivate::enumConstObjects(p_ptr->m_prefab->actor(), prefabs);

            typedef unordered_map<uint32_t, const Object *> ObjectMap;
            ObjectMap cache;
            for(auto it : prefabs) {
                cache[it->uuid()] = it;
            }
            VariantList list;

            ActorPrivate::ConstList objects;
            ActorPrivate::enumConstObjects(this, objects);

            ActorPrivate::ConstList temp = prefabs;
            for(auto obj : objects) {
                auto it = temp.begin();
                while(it != temp.end()) {
                    const Object *o = *it;
                    if(o->uuid() == obj->clonedFrom() || obj->clonedFrom() == 0) {
                        it = temp.erase(it);
                        break;
                    }
                    ++it;
                }
            }
            {
                VariantList list;
                for(auto it : temp) {
                    list.push_back(it->uuid());
                }
                if(!list.empty()) {
                    result[DELETED] = list;
                }
            }

            VariantList fixed;

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
                                            prop[rp.name()] = static_cast<int32_t>(ro->uuid());
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

                    VariantList array;
                    array.push_back(cloned);
                    array.push_back(it->uuid());

                    fixed.push_back(array);
                }
            }

            if(!list.empty()) {
                result[DATA] = list;
            }

            if(!fixed.empty()) {
                result[STATIC] = fixed;
            }
        } else {
            result = p_ptr->m_data;
        }
    }

    return result;
}
