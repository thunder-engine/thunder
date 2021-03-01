#include "core/objectsystem.h"

#include "core/object.h"
#include "core/invalid.h"
#include "core/uri.h"
#include "core/bson.h"
#include "core/json.h"

#include "math/amath.h"

static ObjectSystem::FactoryMap s_Factories;
static ObjectSystem::GroupMap   s_Groups;

/*!
    \class ObjectSystem
    \brief The ObjectSystem responds for object management.
    \since Next 1.0
    \inmodule Core

    ObjectSystem helps to developers create new instances and serialize/deserialize them on disc or in memory.
*/
/*!
    \typedef ObjectSystem::FactoryMap

    This container holds all registered Objects which can be easily created through objectCreate().

    \sa objectCreate()
*/
/*!
    \typedef ObjectSystem::GroupMap

    This container holds links between objects and groups. Groups helps to manage objects and not used directly.

    \sa objectCreate()
*/
/*!
    \fn void ObjectSystem::factoryAdd(const string &group, const MetaObject *meta)

    Registers class with T type, \a meta object and \a group to object instantiation mechanism.
    \note New classes inherited from base Object class can be automaticaly registered using T::registerClassFactory().
    This is preferable way to use this functionality.
*/
/*!
    \fn void ObjectSystem::factoryRemove(const string &group)

    Unregisters class with type T and \a group from object instantiation mechanism.
    \note The preferable way to use this function is T::unregisterClassFactory() invocation.
*/
/*!
    \fn T *ObjectSystem::objectCreate(const string &name = string(), Object *parent = 0)

    Returns new instance of type T and \a name as child of \a parent object.
    \note Class T should be registered first via factoryAdd()

    \sa factoryAdd(), factoryRemove()
*/
/*!
    Constructs ObjectSystem.
*/
ObjectSystem::ObjectSystem() :
        m_SuspendObject(nullptr) {
    PROFILE_FUNCTION();
}
/*!
    Destructs ObjectSystem, related objects and registered object factories.
*/
ObjectSystem::~ObjectSystem() {
    PROFILE_FUNCTION();

    {
        auto it = s_Factories.begin();
        while(it != s_Factories.end()) {
            FactoryPair &pair = it->second;
            if(pair.second == this) {
                s_Groups.erase(pair.first->name());
                it = s_Factories.erase(it);
                continue;
            }
            it++;
        }
    }
    {
        deleteAllObjects();
        m_SuspendObject = nullptr;
    }
}
/*!
    Updates all related objects.
*/
void ObjectSystem::processEvents() {
    PROFILE_FUNCTION();

    Object::processEvents();

    auto it = m_ObjectList.begin();
    while(it != m_ObjectList.end()) {
        Object *o = *it;
        o->processEvents();

        if(m_SuspendObject != nullptr) {
            m_SuspendObject = nullptr;
            it = m_ObjectList.erase(it);
        } else {
            ++it;
        }
    }
}
/*!
    Returns new instance of type represented in \a uri and \a name as child of \a parent object.
    \note Class represented as uri should be registered first via factoryAdd()

    \sa factoryAdd(), factoryRemove()
*/
Object *ObjectSystem::objectCreate(const string &uri, const string &name, Object *parent) {
    PROFILE_FUNCTION();

    Object *object = nullptr;
    FactoryPair *pair = metaFactory(uri);
    if(pair) {
        const MetaObject *meta = pair->first;
        object = pair->second->instantiateObject(meta);
        object->setSystem(pair->second);
        object->setType(uri);
        object->setName(name);
        object->setParent(parent);
    }
    return object;
}
/*!
    The basic method to spawn a new object based on the provided \a meta object.
    Returns a pointer to spawned object.
*/
Object *ObjectSystem::instantiateObject(const MetaObject *meta) {
    return meta->createInstance();
}
/*!
    \internal
*/
void ObjectSystem::factoryAdd(const string &name, const string &uri, const MetaObject *meta) {
    PROFILE_FUNCTION();
    s_Groups[name]   = uri;
    s_Factories[uri] = FactoryPair(meta, this);
}
/*!
    \internal
*/
void ObjectSystem::factoryRemove(const string &name, const string &uri) {
    PROFILE_FUNCTION();
    s_Groups.erase(name);
    s_Factories.erase(uri);
}
/*!
    \internal
*/
void ObjectSystem::deleteAllObjects() {
    auto it = m_ObjectList.begin();
    while(it != m_ObjectList.end()) {
        delete *it;
        it = m_ObjectList.begin();
    }
}
/*!
    Returns all registered classes.
*/
ObjectSystem::GroupMap ObjectSystem::factories() const {
    PROFILE_FUNCTION();
    return s_Groups;
}
/*!
    Returns MetaObject for registered factory by provided \a uri.
*/
ObjectSystem::FactoryPair *ObjectSystem::metaFactory(const string &uri) {
    PROFILE_FUNCTION();
    FactoryMap::iterator it = s_Factories.find(uri);
    if(it == s_Factories.end()) {
        it  = s_Factories.find(s_Groups[uri]);
    }
    if(it != s_Factories.end()) {
        return &((*it).second);
    }
    return nullptr;
}

typedef list<const Object *> ObjectArray;
void enumObjects(const Object *object, ObjectArray &list) {
    PROFILE_FUNCTION();
    list.push_back(object);
    for(const auto &it : object->getChildren()) {
        enumObjects(it, list);
    }
}
/*!
    Returns serialized to Variant version of \a object inherited from Object class.
    This method saves all object property values, active connections and necessary parameters.
    \note All childs of object will be also serialized.

    The returned value can be saved on disk in BSON or JSON form or keep it in memory.
    Developers is able to save own data using Object::saveUserData() mechanism.
*/
Variant ObjectSystem::toVariant(const Object *object) {
    PROFILE_FUNCTION();
    VariantList result;

    ObjectArray list;
    enumObjects(object, list);

    for(auto it : list) {
        // Save Object
        if(!it->isSerializable()) {
            continue;
        }
        result.push_back(it->saveData());
    }

    return result;
}
/*!
    Returns object deserialized from \a variant based representation.
    The Variant representation can be loaded from BSON or JSON formats or retrieved from memory.
    Deserialization will try to restore objects hierarchy with \a root as parent, its properties and connections.
*/
Object *ObjectSystem::toObject(const Variant &variant, Object *root) {
    PROFILE_FUNCTION();
    Object *result  = nullptr;

    typedef unordered_map<uint32_t, Object *> ObjectMap;
    ObjectMap array;

    // Create all declared objects
    VariantList objects = variant.value<VariantList>();
    for(auto it : objects) {
        VariantList o  = it.value<VariantList>();
        if(o.size() >= 5) {
            auto i = o.begin();
            string type = (*i).toString();
            i++;
            uint32_t uuid = static_cast<uint32_t>((*i).toInt());
            i++;

            Object *parent = root;
            for(auto item : array) {
                Object *obj = findObject(static_cast<uint32_t>((*i).toInt()), item.second);
                if(obj) {
                    parent = obj;
                    break;
                }
            }

            i++;
            string name = (*i).toString();
            i++;

            Object *object = objectCreate(type, name, parent);
            if(object) {
                object->setUUID(uuid);
                array[uuid] = object;
            } else {
                // Create a dummy object to keep all fields
                Invalid *invalid = new Invalid();
                invalid->loadData(o);
                object = invalid;

                object->setUUID(uuid);
                object->setName(name);
                object->setParent(parent);

                array[uuid] = object;
            }

            i++;
            i++;
            // Load user data
            VariantMap &user = *(reinterpret_cast<VariantMap *>((*i).data()));
            object->loadObjectData(user);

            if(result == nullptr && object->parent() == root) {
                result = object;
            }
        }
    }

    for(auto it : objects) {
        VariantList &o  = *(reinterpret_cast<VariantList *>(it.data()));
        if(o.size() >= 5) {
            auto i = o.begin();
            i++;
            uint32_t uuid = static_cast<uint32_t>((*i).toInt());
            i++;
            i++;
            i++;

            Object *object = nullptr;
            auto ot  = array.find(uuid);
            if(ot != array.end()) {
                object = (*ot).second;
            } else {
                return nullptr;
            }

            // Load base properties
            VariantMap &properties = *(reinterpret_cast<VariantMap *>((*i).data()));
            for(const auto &prop : properties) {
                Variant v  = prop.second;
                if(v.type() < MetaType::USERTYPE) {
                    object->setProperty(prop.first.c_str(), v);
                }
            }
            i++;
            // Restore connections
            VariantList &links = *(reinterpret_cast<VariantList *>((*i).data()));
            for(const auto &link : links) {
                VariantList &list = *(reinterpret_cast<VariantList *>(link.data()));
                Object *sender = nullptr;
                Object *receiver = nullptr;
                if(list.size() == 4) {
                    auto l  = list.begin();
                    auto s  = array.find(static_cast<uint32_t>((*l).toInt()));
                    if(s != array.end()) {
                        sender  = (*s).second;
                    }
                    l++;

                    string signal = (*l).toString();
                    l++;

                    s = array.find(static_cast<uint32_t>((*l).toInt()));
                    if(s != array.end()) {
                        receiver  = (*s).second;
                    }
                    l++;

                    string method = (*l).toString();
                    l++;

                    connect(sender, signal.c_str(), receiver, method.c_str());
                }
            }

            i++;
            // Load user data
            VariantMap &user = *(reinterpret_cast<VariantMap *>((*i).data()));
            object->loadUserData(user);
        }
    }

    return result;
}
/*!
    Returns the new unique ID based on random number generator.
*/
uint32_t ObjectSystem::generateUUID() {
    PROFILE_FUNCTION();
    return dist(mt);
}
/*!
    Replaces current \a uuid of the \a object with the new one.
*/
void ObjectSystem::replaceUUID(Object *object, uint32_t uuid) {
    PROFILE_FUNCTION();
    if(object) {
        object->setUUID(uuid);
    }
}
/*!
    Returns root \a object in the hierarchy.
*/
Object *ObjectSystem::findRoot(Object *object) {
    Object *root = object;
    while(true) {
        Object *parent = root->parent();
        if(parent) {
            root = parent;
        } else {
            break;
        }
    }
    return root;
}
/*!
    Returns object with \a uuid or which was clonned from this.
    This algorithm recursively going down from the \a root object
    If the object doesn't exist in the hierarchy this method returns nullptr.
*/
Object *ObjectSystem::findObject(uint32_t uuid, Object *root) {
    if(root->clonedFrom() == uuid || root->uuid() == uuid) {
        return root;
    }
    for(auto &it : root->getChildren()) {
        Object *result = findObject(uuid, it);
        if(result) {
            return result;
        }
    }
    return nullptr;
}
/*!
    Adds an \a object to main pull of objects in ObjectSystem
*/
void ObjectSystem::addObject(Object *object) {
    PROFILE_FUNCTION();
    m_ObjectList.push_back(object);
}
/*!
    \internal
*/
void ObjectSystem::removeObject(Object *object) {
    PROFILE_FUNCTION();
    if(m_SuspendObject == nullptr) {
        m_ObjectList.remove(object);
    }
}
/*!
    \internal
*/
void ObjectSystem::suspendObject(Object *object) {
    m_SuspendObject = object;
}
