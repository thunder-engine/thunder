#include "components/actor.h"
#include "components/scene.h"
#include "components/transform.h"

#include "resources/prefab.h"

#include "systems/resourcesystem.h"

#include "commandbuffer.h"

#include <cstring>

const char *PREFAB  ("Prefab");
const char *DATA    ("PrefabData");
const char *STATIC  ("Static");
const char *DELETED ("Deleted");
const char *TRANSFORM("Transform");

class ActorPrivate : public Resource::IObserver {
public:
    explicit ActorPrivate(Actor *actor) :
        m_pTransform(nullptr),
        m_pPrefab(nullptr),
        m_pScene(nullptr),
        m_pActor(actor),
        m_Layers(ICommandBuffer::DEFAULT | ICommandBuffer::RAYCAST | ICommandBuffer::SHADOWCAST| ICommandBuffer::TRANSLUCENT),
        m_Enable(true) {

    }

    ~ActorPrivate() {
        if(m_pPrefab) {
            m_pPrefab->unsubscribe(this);
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) {
        if(resource == m_pPrefab) {
            switch(state) {
                case Resource::Loading: {
                    m_Data = m_pActor->saveUserData();
                }
                case Resource::Ready: {
                    ActorPrivate::ConstList prefabs;
                    ActorPrivate::enumConstObjects(m_pPrefab->actor(), prefabs);

                    ActorPrivate::List objects;
                    ActorPrivate::enumObjects(m_pActor, objects);

                    list<pair<const Object *, Object *>> array;

                    ActorPrivate::List deleteObjects = objects;
                    for(auto obj : prefabs) {
                        bool create = true;
                        auto it = deleteObjects.begin();
                        while(it != deleteObjects.end()) {
                            Object *o = *it;
                            if(obj->uuid() == o->clonedFrom() || o->clonedFrom() == 0) {
                                array.push_back(pair<const Object *, Object *>(obj, o));
                                it = deleteObjects.erase(it);
                                create = false;
                                break;
                            }
                            ++it;
                        }
                        if(create) {
                            Object *parent = System::findObject(obj->parent()->uuid(), m_pActor);
                            Object *result = Engine::objectCreate(obj->typeName(), obj->name(), parent);

                            array.push_back(pair<const Object *, Object *>(obj, result));
                        }
                    }

                    for(auto it : array) {
                        const MetaObject *meta = it.first->metaObject();

                        for(int i = 0; i < meta->propertyCount(); i++) {
                            MetaProperty rp = meta->property(i);
                            MetaProperty lp = it.second->metaObject()->property(i);
                            Variant data = rp.read(it.first);
                            if(rp.type().flags() & MetaType::BASE_OBJECT) {
                                Object *ro = *(reinterpret_cast<Object **>(data.data()));

                                for(auto item : array) {
                                    if(item.first == ro) {
                                        ro = item.second;
                                        break;
                                    }
                                }

                                data = Variant(data.userType(), &ro);
                            }
                            lp.write(it.second, data);
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

                    m_pActor->loadUserData(m_Data.toMap());
                } break;
                case Resource::ToBeDeleted: {
                    m_pPrefab = nullptr;
                } break;
                default: break;
            }
        }
    }

    static bool isPointer(const char *name) {
        for(uint32_t i = 0; i < strlen(name); i++) {
            if(name[i] == '*') {
                return true;
            }
        }
        return false;
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

    Variant m_Data;

    Transform *m_pTransform;

    Prefab *m_pPrefab;

    Scene *m_pScene;

    Actor *m_pActor;

    int32_t m_Layers;

    bool m_Enable;
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
        p_ptr(new ActorPrivate(this)) {

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
    Marks this Actor as \a enabled or disabled.
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
int Actor::layers() const {
    PROFILE_FUNCTION();
    return p_ptr->m_Layers;
}
/*!
    Assigns the list of \a layers for this Actor as a bitmask.
*/
void Actor::setLayers(const int layers) {
    PROFILE_FUNCTION();
    p_ptr->m_Layers = layers;
}
/*!
    Returns the Transform component attached to this Actor.
    If no Transform component found this method will create a new one.
*/
Transform *Actor::transform() {
    PROFILE_FUNCTION();
    if(p_ptr->m_pTransform == nullptr) {
        p_ptr->m_pTransform = fetchTransform();
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
    Tries to find a Transform in components.
    In case of failure, it will create a new one.
*/
Transform *Actor::fetchTransform() {
    Transform *result = static_cast<Transform *>(component(TRANSFORM));
    if(result == nullptr) {
        result = static_cast<Transform *>(addComponent(TRANSFORM));
    }
    return result;
}
/*!
    \internal
*/
Object *Actor::clone(Object *parent) {
    PROFILE_FUNCTION();
    ActorPrivate::List objects;
    ActorPrivate::enumObjects(this, objects);
    for(auto it : objects) {
        Actor *actor = dynamic_cast<Actor *>(it);
        if(actor) {
            actor->transform();
        }
    }

    Actor *result = static_cast<Actor *>(Object::clone(parent));
    Prefab *prefab = dynamic_cast<Prefab *>(Object::parent());
    if(prefab) {
        result->setPrefab(prefab);
    } else {
        result->setPrefab(p_ptr->m_pPrefab);
    }
    return result;
}
/*!
    \internal
*/
void Actor::clearCloneRef () {
    PROFILE_FUNCTION();
    if(p_ptr->m_pPrefab == nullptr) {
        Object::clearCloneRef();

        for(auto it : getChildren()) {
            it->clearCloneRef();
        }
    }
}
/*!
    Makes the actor a child of the \a parent.
    \note Please ignore the \a force flag it will be provided by the default.
*/
void Actor::setParent(Object *parent, bool force) {
    PROFILE_FUNCTION();
    if(p_ptr->m_pTransform) {
        Object::setParent(parent, force);

        Actor *actor = dynamic_cast<Actor *>(parent);
        if(actor) {
            p_ptr->m_pTransform->setParentTransform(actor->transform(), force);
        }
    } else {
        Object::setParent(parent);
    }
}
/*!
    Returns true in case the current object is an instance of the serialized prefab structure; otherwise returns false.
*/
bool Actor::isInstance() const {
    PROFILE_FUNCTION();
    return (p_ptr->m_pPrefab != nullptr);
}
/*!
    Marks this Actor as an instance of the \a prefab structure.
    \note This method is used for internal purposes and shouldn't be called manually.

    \internal
*/
void Actor::setPrefab(Prefab *prefab) {
    PROFILE_FUNCTION();
    if(p_ptr->m_pPrefab) {
        p_ptr->m_pPrefab->unsubscribe(p_ptr);
    }
    p_ptr->m_pPrefab = prefab;
    if(p_ptr->m_pPrefab) {
        p_ptr->m_pPrefab->subscribe(p_ptr);
    } else {
        clearCloneRef();
    }
}
/*!
    \internal
*/
void Actor::loadObjectData(const VariantMap &data) {
    PROFILE_FUNCTION();
    ResourceSystem *system = static_cast<ResourceSystem *>(Engine::resourceSystem());

    auto it = data.find(PREFAB);
    if(it != data.end()) {
        setPrefab(dynamic_cast<Prefab *>(system->loadResource((*it).second.toString())));

        if(p_ptr->m_pPrefab) {
            Actor *actor = static_cast<Actor *>(p_ptr->m_pPrefab->actor()->clone());

            it = data.find(DELETED);
            if(it != data.end()) {
                for(auto item : (*it).second.toList()) {
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
                for(auto item : (*it).second.toList()) {
                    VariantList array = item.toList();

                    uint32_t clone = static_cast<uint32_t>(array.front().toInt());
                    Object *result = ObjectSystem::findObject(clone, this);
                    if(result) {
                        ObjectSystem::replaceUUID(result, static_cast<uint32_t>(array.back().toInt()));
                    }
                }
            }
        }
    }
}
/*!
    \internal
*/
void Actor::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();
    ResourceSystem *system = static_cast<ResourceSystem *>(Engine::resourceSystem());

    if(p_ptr->m_pPrefab) {
        auto it = data.find(DATA);
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
                                    Object *res = system->loadResource(var.toString());
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

    ResourceSystem *system = static_cast<ResourceSystem *>(Engine::resourceSystem());
    if(p_ptr->m_pPrefab) {
        string ref = system->reference(p_ptr->m_pPrefab);
        if(!ref.empty()) {
            result[PREFAB] = ref;

            ActorPrivate::ConstList prefabs;
            ActorPrivate::enumConstObjects(p_ptr->m_pPrefab->actor(), prefabs);

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

                                    string lref = system->reference(lo);
                                    string rref = system->reference(ro);
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
        }
    }

    return result;
}
