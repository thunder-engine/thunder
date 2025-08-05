#include "components/actor.h"
#include "components/scene.h"
#include "components/transform.h"
#include "components/world.h"

#include "resources/prefab.h"

#include "systems/resourcesystem.h"

#include "commandbuffer.h"

#include <cstring>

namespace {
    const char *gFlags("Flags");
    const char *gPrefab("Prefab");
    const char *gData("PrefabData");
    const char *gStatic("Static");
    const char *gDeleted("Deleted");
    const char *gTransform("Transform");
}

static void enumConstObjects(const Object *object, Prefab::ConstObjectList &list) {
    PROFILE_FUNCTION();
    list.push_back(object);
    for(const auto &it : object->getChildren()) {
        enumConstObjects(it, list);
    }
}

static Component *componentInChildHelper(const TString &type, Object *parent) {
    PROFILE_FUNCTION();
    for(auto it : parent->getChildren()) {
        const MetaObject *meta = it->metaObject();
        if(meta->canCastTo(type.data())) {
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
        m_flags(Actor::Enable | Actor::Selectable),
        m_hierarchyEnable(m_flags & Actor::Enable),
        m_muteUpdates(false) {

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
    return m_flags & Enable;
}
/*!
    Marks this Actor as \a enabled or disabled.
    Disabled Actors becomes invisible for the user.
*/
void Actor::setEnabled(const bool enabled) {
    PROFILE_FUNCTION();
    if(enabled) {
        m_flags |= Enable;
    } else {
        m_flags &= ~Enable;
    }

    setHierarchyEnabled(enabled);

    if(m_transform) {
        m_transform->setEnabled(enabled);
    }
}
/*!
    Returns a set of Actor::Flags applied to this Actor.
*/
int Actor::flags() const {
    PROFILE_FUNCTION();

    return m_flags;
}
/*!
    Applies a new set of Actor::Flags \a flags to this Actor.
*/
void Actor::setFlags(int flags) {
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
    return m_flags & Static;
}
/*!
    Marks current Actor as static or dynamic (by default).
    This \a flag can help to optimize rendering.
*/
void Actor::setStatic(const bool flag) {
    if(flag) {
        m_flags |= Static;
    } else {
        m_flags &= ~Static;
    }
}
/*!
    Returns the Transform component attached to this Actor.
*/
Transform *Actor::transform() {
    PROFILE_FUNCTION();
    if(m_transform == nullptr) {
        setTransform(getComponent<Transform>());
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
Component *Actor::component(const TString &type) {
    PROFILE_FUNCTION();
    for(auto it : getChildren()) {
        const MetaObject *meta = it->metaObject();
        if(meta->canCastTo(type.data())) {
            return static_cast<Component *>(it);
        }
    }
    return nullptr;
}
/*!
    Returns the component with \a type in the Actor's children using depth search.
    A component is returned only if it's found on a current Actor; otherwise returns nullptr.
*/
Component *Actor::componentInChild(const TString &type) {
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
std::list<Component *> Actor::componentsInChild(const TString &type) const {
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
Component *Actor::addComponent(const TString &type) {
    PROFILE_FUNCTION();
    return static_cast<Component *>(Engine::objectCreate(type, type, this));
}
/*!
    \internal
*/
bool Actor::isSerializable() const {
    PROFILE_FUNCTION();
    return !(m_flags & NonSerializable) && (clonedFrom() == 0 || isInstance());
}
/*!
    \internal
*/
Object *Actor::cloneStructure(ObjectPairs &pairs) {
    PROFILE_FUNCTION();
    Actor *result = static_cast<Actor *>(Object::cloneStructure(pairs));
    Prefab *prefab = dynamic_cast<Prefab *>(Object::parent());
    if(prefab == nullptr) {
        prefab = m_prefab;
    }

    result->setPrefab(prefab);

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
    If \a force is true the parent-relative position, scale and rotation recalculation will be ignored.
*/
void Actor::setParent(Object *parent, int32_t position, bool force) {
    PROFILE_FUNCTION();
    if(parent == this || (Object::parent() == parent && position == -1)) {
        return;
    }

    Actor *actor = dynamic_cast<Actor *>(parent);
    if(actor) {
        setScene(actor->scene());
        m_hierarchyEnable = actor->m_hierarchyEnable;
    } else {
        setScene(dynamic_cast<Scene *>(parent));
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
        }

        m_prefab = prefab;
        if(m_prefab) {
            m_muteUpdates = true;
            m_prefab->subscribe(&Actor::prefabUpdated, this);
            m_muteUpdates = false;
        } else {
            clearCloneRef();
        }
    }
}

Object *findByClone(uint32_t uuid, Object *root) {
    if(root) {
        if(root->clonedFrom() == uuid) {
            return root;
        }
        for(auto &it : root->getChildren()) {
            Object *result = findByClone(uuid, it);
            if(result) {
                return result;
            }
        }
    }
    return nullptr;
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

                    // Need to do search by cloneID!
                    Object *result = findByClone(uuid, actor);
                    if(result && result != actor) {
                        delete result;
                    }
                }
            }

            Object::ObjectList children = actor->getChildren(); // Need to copy a list
            for(auto &it : children) {
                it->setParent(this);
            }
            ObjectSystem::replaceClonedUUID(this, actor->clonedFrom());
            delete actor;

            auto it = data.find(gStatic);
            if(it != data.end()) {
                for(auto &item : (*it).second.toList()) {
                    VariantList array = item.toList();

                    uint32_t originID = static_cast<uint32_t>(array.front().toInt());
                    // Need to do search by cloneID!
                    Object *result = findByClone(originID, this);
                    if(result) {
                        uint32_t newID = static_cast<uint32_t>(array.back().toInt());
                        ObjectSystem::replaceUUID(result, newID);
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

    it = data.find(gData);
    if(it != data.end()) {
        Object::ObjectList objects;
        Object::enumObjects(this, objects);

        ObjectSystem::ObjectMap cacheMap;
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
                    int32_t index = meta->indexOfProperty(property.first.data());
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

        Object *obj = Engine::findObject(uuid);
        if(obj == nullptr) {
            obj = Engine::findObject(uuid);
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
        TString ref = Engine::reference(m_prefab);
        if(!ref.isEmpty()) {
            result[gPrefab] = ref;

            VariantList list;

            Prefab::ConstObjectList objects;
            enumConstObjects(this, objects);

            VariantList deletedList;
            for(auto it : m_prefab->absentInCloned(objects)) {
                deletedList.push_back(it->uuid());
            }
            if(!deletedList.empty()) {
                result[gDeleted] = deletedList;
            }

            VariantList fixed;

            for(auto it : objects) {
                uint32_t cloned = it->clonedFrom();
                if(cloned) {
                    Object *proto = m_prefab->protoObject(cloned);
                    if(proto) {
                        VariantMap prop;

                        const MetaObject *meta = it->metaObject();
                        int count  = meta->propertyCount();
                        for(int i = 0; i < count; i++) {
                            MetaProperty lp(proto->metaObject()->property(i));
                            MetaProperty rp(meta->property(i));
                            Variant lv(lp.read(proto));
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

                        fixed.push_back(VariantList({cloned, it->uuid()}));
                    }
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

    TString lref(Engine::reference(lo));
    TString rref(Engine::reference(ro));
    if(lref != rref) {
        return rref;
    }

    if(rref.isEmpty() && lref.isEmpty()) {
        if((lo == nullptr && ro) || (ro && lo->uuid() != ro->uuid())) {
            return static_cast<int32_t>(ro->uuid());
        }
    }

    return Variant();
}

void Actor::prefabUpdated(int state, void *ptr) {
    Actor *p = static_cast<Actor *>(ptr);

    if(p->m_muteUpdates) {
        return;
    }

    switch(state) {
        case Resource::Loading: {
            p->m_data = p->saveUserData();
        } break;
        case Resource::Ready: {
            p->m_transform = nullptr; // What the reason?

            ObjectList objects;
            Engine::enumObjects(p, objects);

            ObjectPairs pairs;

            Prefab::ConstObjectList objectsList;
            Prefab::ConstObjectList objectsToDelete;
            for(auto &it : objects) {
                uint32_t originID = it->clonedFrom();
                if(originID != 0) {
                    Object *protoObject = p->m_prefab->protoObject(originID);
                    if(protoObject) {
                        pairs.push_back(std::make_pair(protoObject, it));
                        objectsList.push_back(it);
                    } else {
                        objectsToDelete.push_back(it);
                    }
                }

            }

            // New objects
            objects = p->m_prefab->absentInCloned(objectsList);
            for(auto it : objects) {
                static_cast<Actor *>(it)->cloneStructure(pairs);
            }

            Actor::syncProperties(p, pairs);

            objectsToDelete.reverse();
            for(auto it : objectsToDelete) {
                delete it;
            }

            p->loadUserData(p->m_data);
        } break;
        case Resource::ToBeDeleted: {
            for(auto &it : p->getChildren()) {
                if(p->m_prefab->contains(it->clonedFrom())) {
                    it->deleteLater();
                }
            }
            p->m_data = p->saveUserData();
            p->m_prefab = nullptr;
        } break;
        default: break;
    }
}
