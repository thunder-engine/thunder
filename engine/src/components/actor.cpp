#include "components/actor.h"
#include "components/scene.h"
#include "components/transform.h"
#include "components/world.h"

#include "resources/prefab.h"

#include "systems/resourcesystem.h"

#include "commandbuffer.h"

#include <cstring>

namespace {
    const char *gFlags = "Flags";
    const char *gPrefab = "Prefab";
    const char *gData = "PrefabData";
    const char *gStatic = "Static";
    const char *gDeleted = "Deleted";
    const char *gTransform = "Transform";
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

/*!
    \class Actor
    \brief Base class for all entities in Thunder Engine.
    \inmodule Components

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
        m_transform(nullptr),
        m_prefab(nullptr),
        m_scene(nullptr),
        m_layers(CommandBuffer::DEFAULT | CommandBuffer::RAYCAST | CommandBuffer::SHADOWCAST | CommandBuffer::TRANSLUCENT),
        m_flags(Actor::ENABLE | Actor::SELECTABLE),
        m_hierarchyEnable(m_flags & Actor::ENABLE),
        m_static(false) {

}

Actor::~Actor() {
    if(m_prefab) {
        m_prefab->unsubscribe(this);
    }

    if(m_scene && m_scene->world()) {
        m_scene->world()->makeDirty();
    }
}
/*!
    Returns true in case of Actor is enabled; otherwise returns false.
    Disabled Actors becomes invisible for the user.
    By default the property is \b true.
*/
bool Actor::isEnabled() const {
    PROFILE_FUNCTION();
    return m_flags & ENABLE;
}
/*!
    Marks this Actor as \a enabled or disabled.
    Disabled Actors becomes invisible for the user.
*/
void Actor::setEnabled(const bool enabled) {
    PROFILE_FUNCTION();
    if(enabled) {
        m_flags |= ENABLE;
    } else {
        m_flags &= ~ENABLE;
    }

    setHierarchyEnabled(enabled);
}
/*!
    Returns a set of Actor::HideFlags applied to this Actor.
*/
int Actor::hideFlags() const {
    PROFILE_FUNCTION();

    return m_flags;
}
/*!
    Applies a new set of Actor::HideFlags \a flags to this Actor.
*/
void Actor::setHideFlags(int flags) {
    PROFILE_FUNCTION();

    bool old = isEnabled();

    m_flags = flags;

    bool current = isEnabled();
    if(old != current) {
        setHierarchyEnabled(current);
    }
}
/*!
    Returns false in case of one of Actors in top hierarchy was disabled; otherwise returns true.
*/
bool Actor::isEnabledInHierarchy() const {
    return (m_hierarchyEnable && isEnabled());
}
/*!
    \internal
*/
void Actor::setHierarchyEnabled(bool enabled) {
    m_hierarchyEnable = enabled;
    for(auto it : getChildren()) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor && actor->isEnabled()) {
            actor->setHierarchyEnabled(enabled);
        }
    }
}
/*!
    \internal
*/
void Actor::setScene(Scene *scene) {
    if(m_scene != scene) {
        m_scene = scene;
        if(m_scene && m_scene->world()) {
            m_scene->world()->makeDirty();
        }
        for(auto it : getChildren()) {
            Actor *child = dynamic_cast<Actor *>(it);
            if(child) {
                child->setScene(scene);
            }
        }
    }
}
/*!
    Returns true if this actor will not be moved during the game; otherwise returns false.
*/
bool Actor::isStatic() const {
    return m_static;
}
/*!
    Marks current Actor as static or dynamic (by default).
    This \a flag can help to optimize rendering.
*/
void Actor::setStatic(const bool flag) {
    m_static = flag;
}
/*!
    Returns the layers list for the this Actor as a bit mask.
    The layers used for the various purposes like filtering objects before rendering.
*/
int Actor::layers() const {
    PROFILE_FUNCTION();
    return m_layers;
}
/*!
    Assigns the list of \a layers for this Actor as a bitmask.
*/
void Actor::setLayers(const int layers) {
    PROFILE_FUNCTION();
    m_layers = layers;
}
/*!
    Returns the Transform component attached to this Actor.
*/
Transform *Actor::transform() {
    PROFILE_FUNCTION();
    if(m_transform == nullptr) {
        setTransform(static_cast<Transform *>(component(gTransform)));
    }
    return m_transform;
}
/*!
    Replaces an existant \a transform with new one.
*/
void Actor::setTransform(Transform *transform) {
    m_transform = transform;
}
/*!
    Returns the scene where actor attached to.
*/
Scene *Actor::scene() const {
    PROFILE_FUNCTION();
    return m_scene;
}
/*!
    Returns the world where actor attached to.
*/
World *Actor::world() const{
    PROFILE_FUNCTION();
    if(m_scene) {
        return m_scene->world();
    }
    return nullptr;
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
        Component *result = componentInChildHelper(type, it);
        if(result) {
            return result;
        }
    }
    return nullptr;
}
/*!
    Returns a list of the components with \a type in the Actor's children using depth search.
*/
std::list<Component *> Actor::componentsInChild(const string type) {
    PROFILE_FUNCTION();
    std::list<Component *> result;
    for(auto it : getChildren()) {
        Component *component = componentInChildHelper(type, it);
        if(component) {
            result.push_back(component);
        }
    }
    return result;
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
        result->setPrefab(m_prefab);
    }
    return result;
}
/*!
    \internal
*/
void Actor::clearCloneRef() {
    PROFILE_FUNCTION();
    if(m_prefab == nullptr) {
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

    m_scene = nullptr;

    Actor *actor = dynamic_cast<Actor *>(parent);
    if(actor) {
        setScene(actor->scene());
        m_hierarchyEnable = actor->m_hierarchyEnable;
    } else {
        Scene *scene = dynamic_cast<Scene *>(parent);
        if(scene) {
            setScene(scene);
        }
    }
    if(m_transform) {
        Object::setParent(parent, position, force);
        if(actor) {
            m_transform->setParentTransform(actor->transform(), force);
        }
    } else {
        Object::setParent(parent, position);
    }

    for(auto it : getChildren()) {
        Component *component = dynamic_cast<Component *>(it);
        if(component) {
            component->actorParentChanged();
        }
    }
}
/*!
    Returns true in case the current object is an instance of the serialized prefab structure; otherwise returns false.
*/
bool Actor::isInstance() const {
    PROFILE_FUNCTION();
    return (m_prefab != nullptr);
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
    return m_prefab;
}
/*!
    Marks this Actor as an instance of the \a prefab structure.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
void Actor::setPrefab(Prefab *prefab) {
    PROFILE_FUNCTION();

    if(m_prefab != prefab) {
        if(m_prefab) {
            m_prefab->unsubscribe(this);
            m_prefab->decRef();
        }

        m_prefab = prefab;
        if(m_prefab) {
            m_prefab->incRef();
            m_prefab->subscribe(&Actor::prefabUpdated, this);
        } else {
            clearCloneRef();
        }
    }
}
/*!
    \internal
*/
void Actor::loadObjectData(const VariantMap &data) {
    PROFILE_FUNCTION();

    auto it = data.find(gPrefab);
    if(it != data.end()) {
        setPrefab(Engine::loadResource<Prefab>((*it).second.toString()));

        if(m_prefab) {
            Actor *actor = static_cast<Actor *>(m_prefab->actor()->clone());

            it = data.find(gDeleted);
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
            auto it = data.find(gStatic);
            if(it != data.end()) {
                for(auto &item : (*it).second.toList()) {
                    VariantList array = item.toList();

                    int32_t clone = static_cast<uint32_t>(array.front().toInt());
                    Object *result = ObjectSystem::findObject(clone, this);
                    if(result) {
                        ObjectSystem::replaceUUID(result, static_cast<uint32_t>(array.back().toInt()));
                    }
                }
            }
        } else {
            m_data = data;
        }
    }
}
/*!
    \internal
*/
void Actor::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();

    auto it = data.find(gFlags);
    if(it != data.end()) {
        m_flags = it->second.toInt();
    }

    if(m_prefab) {
        it = data.find(gData);
        if(it != data.end()) {
            Object::ObjectList objects;
            Object::enumObjects(this, objects);

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
                            bool isObject = prop.type().flags() & MetaType::BASE_OBJECT;
                            Variant var(property.second);
                            if(var.type() == MetaType::VARIANTLIST) {
                                VariantList resultList;
                                for(auto it : var.toList()) {
                                    Variant result(Actor::loadObject(it));
                                    if(!isObject) {
                                        isObject = result.isValid();
                                    }
                                    resultList.push_back(result);
                                }
                                if(isObject) {
                                    var = resultList;
                                }
                            } else {
                                if(isObject) {
                                    var = loadObject(var);
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
Variant Actor::loadObject(Variant &value) {
    if(value.type() == MetaType::STRING) { // Asset
        Object *res = Engine::resourceSystem()->loadResource(value.toString());
        if(res) {
            const char *name = res->metaObject()->name();
            return Variant(MetaType::type(name)+1, &res);
        }
    } else if(value.type() == MetaType::INTEGER) { // Component
        uint32_t uuid = static_cast<uint32_t>(value.toInt());

        Object *obj = Engine::findObject(uuid, this);
        if(obj == nullptr) {
            obj = Engine::findObject(uuid, Engine::findRoot(this));
        }
        if(obj) {
            const char *name = obj->metaObject()->name();
            return Variant(MetaType::type(name)+1, &obj);
        }
    }

    return Variant();
}
/*!
    \internal
*/
VariantMap Actor::saveUserData() const {
    PROFILE_FUNCTION();
    VariantMap result = Object::saveUserData();

    result[gFlags] = m_flags;

    if(isInstance()) {
        string ref = Engine::reference(m_prefab);
        if(!ref.empty()) {
            result[gPrefab] = ref;

            ObjectList prefabs;
            Object::enumObjects(m_prefab->actor(), prefabs);

            typedef unordered_map<uint32_t, const Object *> ObjectMap;
            ObjectMap cache;
            for(auto it : prefabs) {
                cache[it->uuid()] = it;
            }
            VariantList list;

            ConstList objects;
            enumConstObjects(this, objects);

            ObjectList temp = prefabs;
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
                    result[gDeleted] = list;
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
                            MetaProperty lp((*fab).second->metaObject()->property(i));
                            MetaProperty rp(meta->property(i));
                            Variant lv(lp.read((*fab).second));
                            Variant rv(rp.read(it));

                            if(lv != rv) {
                                bool isObject = lp.type().flags() & MetaType::BASE_OBJECT;
                                if(isObject) {
                                    Variant val = saveObject(lv, rv);
                                    if(val.isValid()) {
                                        prop[rp.name()] = val;
                                    }
                                } else if(lv.userType() == MetaType::VARIANTLIST) { // Property is an array
                                    VariantList newList;
                                    MetaType::Table *metaType = nullptr;
                                    for(auto &value : rv.toList()) {
                                        if(metaType == nullptr) {
                                            metaType = MetaType::table(value.userType());
                                            isObject = (metaType && metaType->flags & MetaType::BASE_OBJECT);
                                            if(!isObject) {
                                                break;
                                            }
                                        }
                                        if(isObject) {
                                            newList.push_back(saveObject(Variant(), value));
                                        }
                                    }
                                    if(isObject) {
                                        prop[rp.name()] = newList;
                                    } else {
                                        prop[rp.name()] = rv;
                                    }
                                } else {
                                    prop[rp.name()] = rv;
                                }
                            }
                        }

                        if(!prop.empty()) {
                            list.push_back(VariantList({static_cast<int32_t>(cloned), prop}));
                        }

                    }

                    fixed.push_back(VariantList({cloned, it->uuid()}));
                }
            }

            if(!list.empty()) {
                result[gData] = list;
            }

            if(!fixed.empty()) {
                result[gStatic] = fixed;
            }
        } else {
            result = m_data;
        }
    }

    return result;
}
/*!
    \internal
*/
Variant Actor::saveObject(const Variant &lv, const Variant &rv) const {
    Object *lo = lv.isValid() ? *(reinterpret_cast<Object **>(lv.data())) : nullptr;
    Object *ro = *(reinterpret_cast<Object **>(rv.data()));

    string lref = Engine::reference(lo);
    string rref = Engine::reference(ro);
    if(lref != rref) {
        return rref;
    }

    if(rref.empty() && lref.empty()) {
        if((lo == nullptr && ro) || (ro && lo->uuid() != ro->uuid())) {
            return static_cast<int32_t>(ro->uuid());
        }
    }

    return Variant();
}

void Actor::prefabUpdated(int state, void *ptr) {
    Actor *p = static_cast<Actor *>(ptr);

    switch(state) {
        case Resource::Loading: {
            p->m_data = p->saveUserData();
        } break;
        case Resource::Ready: {
            p->m_transform = nullptr;
            Object::ObjectList prefabObjects;
            Object::enumObjects(p->m_prefab->actor(), prefabObjects);

            Object::ObjectList deleteObjects;
            Object::enumObjects(p, deleteObjects);

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
                    Object *parent = System::findObject(prefabObject->parent()->uuid(), p);
                    Object *result = prefabObject->clone(parent ? parent : p);

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

            deleteObjects.reverse();
            for(auto it : deleteObjects) {
                delete it;
            }

            p->loadUserData(p->m_data);
        } break;
        case Resource::ToBeDeleted: {
            /// \todo Unload prefab related components
            for(auto &it : p->getChildren()) {
                if(it != p->transform()) {
                    it->deleteLater();
                }
            }
            p->m_data = p->saveUserData();
            p->m_prefab = nullptr;
        } break;
        default: break;
    }
}
