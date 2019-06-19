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
ObjectSystem::ObjectSystem() {
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
        auto it = m_List.begin();
        while(it != m_List.end()) {
            delete *it;
            it = m_List.begin();
        }
    }
}

/*!
    Updates all related objects.
*/
void ObjectSystem::update() {
    PROFILE_FUNCTION();

    for(auto it : m_List) {
        it->processEvents();
    }
}
/*!
    Returns new instance of type represented in \a uri and \a name as child of \a parent object.
    \note Class represented as uri should be registered first via factoryAdd()

    \sa factoryAdd(), factoryRemove()
*/
Object *ObjectSystem::objectCreate(const string &uri, const string &name, Object *parent) {
    PROFILE_FUNCTION();

    Object *object  = nullptr;

    FactoryPair *pair = metaFactory(uri);
    if(pair) {
        const MetaObject *meta  = pair->first;

        object = meta->createInstance();
        object->setName(name);
        object->setParent(parent);
        object->setSystem(pair->second);

        pair->second->m_List.push_back(object);
    }
    return object;
}

void ObjectSystem::processObject(Object *object) {
    PROFILE_FUNCTION();
    object->processEvents();
}

void ObjectSystem::factoryAdd(const string &name, const string &uri, const MetaObject *meta) {
    PROFILE_FUNCTION();
    s_Groups[name]   = uri;
    s_Factories[uri] = FactoryPair(meta, this);
}

void ObjectSystem::factoryRemove(const string &name, const string &uri) {
    PROFILE_FUNCTION();
    s_Groups.erase(name);
    s_Factories.erase(uri);
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
    Deserialization will try to restore objects hierarchy, its properties and connections.
*/
Object *ObjectSystem::toObject(const Variant &variant, Object *root) {
    PROFILE_FUNCTION();
    Object *result  = nullptr;

    typedef unordered_map<uint32_t, Object *> ObjectMap;
    ObjectMap array;

    // Create all declared objects
    VariantList objects    = variant.value<VariantList>();
    for(auto it : objects) {
        VariantList o  = it.value<VariantList>();
        if(o.size() >= 5) {
            auto i          = o.begin();
            string type = (*i).toString();
            i++;
            uint32_t uuid   = (*i).toInt();
            i++;
            Object *parent  = root;
            auto a  = array.find((*i).toInt());
            if(a != array.end()) {
                parent  = (*a).second;
            }
            i++;
            string name = (*i).toString();
            i++;

            Object *object  = objectCreate(type, name, parent);
            if(object) {
                object->setUUID(uuid);
                array[uuid] = object;

                // Load base properties
                for(const auto &it : (*i).toMap()) {
                    Variant v  = it.second;
                    if(v.type() < MetaType::USERTYPE) {
                        object->setProperty(it.first.c_str(), v);
                    }
                }
                i++;
                // Restore connections
                for(const auto &link : (*i).value<VariantList>()) {
                    VariantList list    = link.value<VariantList>();
                    Object *sender      = nullptr;
                    Object *receiver    = nullptr;
                    if(list.size() == 4) {
                        auto l  = list.begin();
                        auto s  = array.find((*l).toInt());
                        if(s != array.end()) {
                            sender  = (*s).second;
                        }
                        l++;

                        string signal = (*l).toString();
                        l++;

                        s = array.find((*l).toInt());
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
                object->loadUserData((*i).value<VariantMap>());
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

            if(object->parent() == root) {
                result  = object;
            }
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

void ObjectSystem::removeObject(Object *object) {
    PROFILE_FUNCTION();
    auto it = m_List.begin();
    while(it != m_List.end()) {
        if(*it == object) {
            m_List.erase(it);
            return;
        }
        it++;
    }
}
