/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2023 Evgeniy Prikazchikov
*/

#include "core/objectsystem.h"

#include "core/object.h"
#include "core/invalid.h"
#include "core/uri.h"
#include "core/bson.h"
#include "core/json.h"

#include "math/amath.h"

static ObjectSystem::FactoryMap s_Factories;
static ObjectSystem::GroupMap   s_Groups;
static ObjectSystem::ObjectMap  s_Objects;

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
    \typedef ObjectSystem::FactoryPair
    \internal
*/
/*!
    \fn template<typename T> void ObjectSystem::factoryAdd(const string &group, const MetaObject *meta)

    Registers class with T type, \a meta object and \a group to object instantiation mechanism.
    \note New classes inherited from base Object class can be automaticaly registered using T::registerClassFactory().
    This is preferable way to use this functionality.
*/
/*!
    \fn template<typename T> void ObjectSystem::factoryRemove(const string &group)

    Unregisters class with type T and \a group from object instantiation mechanism.
    \note The preferable way to use this function is T::unregisterClassFactory() invocation.
*/
/*!
    \fn template<typename T> T *ObjectSystem::objectCreate(const string &name = string(), Object *parent = 0)

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

    deleteAllObjects();
}
/*!
    Updates all related objects.
*/
void ObjectSystem::processEvents() {
    PROFILE_FUNCTION();

    m_threadId = std::this_thread::get_id();

    Object::processEvents();

    auto it = m_objectList.begin();
    while(it != m_objectList.end()) {
        Object *o = *it;
        auto result = std::find(m_objectToRemove.begin(), m_objectToRemove.end(), o);
        if(result == m_objectToRemove.end()) {
            o->processEvents();
        }
        ++it;
    }

    if(!m_objectToRemove.empty()) {
        for(auto it : m_objectToRemove) {
            m_objectList.remove(it);
        }
        m_objectToRemove.clear();
    }
}
/*!
    Returns true in case of other \a system execues in the same thread with current system; otherwise returns false.
*/
bool ObjectSystem::compareTreads(ObjectSystem *system) const {
    return m_threadId == system->m_threadId;
}
/*!
    Returns new instance of type represented in \a uri and \a name as child of \a parent object.
    \note Class represented as uri should be registered first via factoryAdd()

    \sa factoryAdd(), factoryRemove()
*/
Object *ObjectSystem::objectCreate(const std::string &uri, const std::string &name, Object *parent) {
    PROFILE_FUNCTION();

    Object *object = nullptr;
    FactoryPair *pair = metaFactory(uri);
    if(pair) {
        const MetaObject *meta = pair->first;
        object = pair->second->instantiateObject(meta, name, parent);
        object->setType(uri);
    }
    return object;
}
/*!
    The basic method to spawn a new object based on the provided \a meta object, \a name of object and \a parent object.
    Returns a pointer to spawned object.
*/
Object *ObjectSystem::instantiateObject(const MetaObject *meta, const std::string &name, Object *parent) {
    Object *object = meta->createInstance();
    object->setSystem(this);
    object->setParent(parent);
    object->setName(name);
    return object;
}
/*!
    \internal
*/
void ObjectSystem::factoryAdd(const std::string &name, const std::string &uri, const MetaObject *meta) {
    PROFILE_FUNCTION();
    s_Groups[name]   = uri;
    s_Factories[uri] = FactoryPair(meta, this);
}
/*!
    \internal
*/
void ObjectSystem::factoryRemove(const std::string &name, const std::string &uri) {
    PROFILE_FUNCTION();
    s_Groups.erase(name);
    s_Factories.erase(uri);
}
/*!
    \internal
*/
void ObjectSystem::deleteAllObjects() {
    for(auto it : m_objectList) {
        auto result = std::find(m_objectToRemove.begin(), m_objectToRemove.end(), it);
        if(result == m_objectToRemove.end()) {
            delete it;
        }
    }
    m_objectList.clear();
    m_objectToRemove.clear();
}
/*!
    Returns all registered classes.
*/
ObjectSystem::GroupMap ObjectSystem::factories() {
    PROFILE_FUNCTION();
    return s_Groups;
}
/*!
    Returns MetaObject for registered factory by provided \a uri.
*/
ObjectSystem::FactoryPair *ObjectSystem::metaFactory(const std::string &uri) {
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

typedef std::list<const Object *> ObjectArray;
void enumConstObjects(const Object *object, ObjectArray &list) {
    PROFILE_FUNCTION();
    list.push_back(object);
    for(const auto &it : object->getChildren()) {
        enumConstObjects(it, list);
    }
}
/*!
    Returns serialized to Variant version of \a object inherited from Object class.
    This method saves all object property values, active connections and necessary parameters.
    \note All childs of object will be also serialized.
    \note Function will ignore Object::isSerializable in case of \a force flag provided.

    The returned value can be saved on disk in BSON or JSON form or keep it in memory.
    Developers is able to save own data using Object::saveUserData() mechanism.
*/
Variant ObjectSystem::toVariant(const Object *object, bool force) {
    PROFILE_FUNCTION();
    VariantList result;

    ObjectArray list;
    enumConstObjects(object, list);

    for(auto it : list) {
        // Save Object
        if(!it->isSerializable() && !force) {
            continue;
        }
        result.push_back(it->saveData());
    }

    return result;
}
/*!
    Returns object deserialized from \a variant based representation.
    The Variant representation can be loaded from BSON or JSON formats or retrieved from memory.
    Deserialization will try to restore objects hierarchy with \a parent, its properties and connections.
    The root object will be created with a \a name in case of this parameter provided.
*/
Object *ObjectSystem::toObject(const Variant &variant, Object *parent, const std::string &name) {
    PROFILE_FUNCTION();
    Object *result = nullptr;

    bool first = true;

    // Create all declared objects
    VariantList objects = variant.value<VariantList>();
    for(auto &it : objects) {
        VariantList o  = it.value<VariantList>();
        if(o.size() >= 5) {
            auto i = o.begin();
            std::string type = (*i).toString();
            i++;
            uint32_t uuid = static_cast<uint32_t>((*i).toInt());
            i++;
            uint32_t parentUuid = static_cast<uint32_t>((*i).toInt());
            i++;

            std::string n = (*i).toString();
            if(first && !name.empty()) {
                n = name;
                first = false;
            }
            i++;

            Object *p = parent;
            Object *obj = findObject(parentUuid);
            if(obj != nullptr) {
                p = obj;
            }

            Object *object = findObject(uuid);
            if(object == nullptr) {
                object = objectCreate(type, n, p);

                if(object && uuid != 0) {
                    replaceUUID(object, uuid);
                }
            }

            if(object == nullptr) {
                // Create a dummy object to keep all fields
                Invalid *invalid = new Invalid();
                invalid->loadData(o);
                object = invalid;
                if(p) {
                    object->setSystem(p->system());
                }
                if(uuid != 0) {
                    replaceUUID(object, uuid);
                }
                object->setName(n);
                object->setParent(p);
            }

            i++;
            i++;
            // Load user data
            VariantMap &user = *(reinterpret_cast<VariantMap *>((*i).data()));
            object->loadObjectData(user);

            if(result == nullptr) {
                result = object;
            }
        }
    }

    for(auto &it : objects) {
        VariantList &o  = *(reinterpret_cast<VariantList *>(it.data()));
        if(o.size() >= 5) {
            auto i = o.begin();
            i++;
            uint32_t uuid = static_cast<uint32_t>((*i).toInt());
            i++;
            i++;
            i++;

            Object *object = findObject(uuid);
            if(object == nullptr) {
                return nullptr;
            }

            // Load base properties
            VariantMap &properties = *(reinterpret_cast<VariantMap *>((*i).data()));
            for(const auto &prop : properties) {
                Variant v  = prop.second;
                uint32_t type = v.type();
                if(type < MetaType::USERTYPE && type != MetaType::VARIANTLIST && type != MetaType::VARIANTMAP) {
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
                    auto l = list.begin();
                    sender = findObject(static_cast<uint32_t>((*l).toInt()));
                    l++;

                    std::string signal = (*l).toString();
                    l++;

                    receiver = findObject(static_cast<uint32_t>((*l).toInt()));
                    l++;

                    std::string method = (*l).toString();
                    l++;

                    connect(sender, signal.c_str(), receiver, method.c_str());
                }
            }

            i++;
            // Load user data
            VariantMap &user = *(reinterpret_cast<VariantMap *>((*i).data()));
            object->loadUserData(user);
            i++;
            // Load dynamic properties
            if(i != o.end()) {
                VariantList &dynamic = *(reinterpret_cast<VariantList *>((*i).data()));
                for(auto &property : dynamic) {
                    VariantList &pair = *(reinterpret_cast<VariantList *>(property.data()));
                    std::string name = pair.front().toString();
                    object->setProperty(name.c_str(), pair.back());
                }
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
    uint32_t result = dist(mt);
    while(s_Objects.find(result) != s_Objects.end()) {
        result = dist(mt);
    }
    return result;
}
/*!
    Replaces current \a uuid of the \a object with the new one.
*/
void ObjectSystem::replaceUUID(Object *object, uint32_t uuid) {
    PROFILE_FUNCTION();
    if(object) {
        uint32_t old = object->uuid();
        auto it = s_Objects.find(old);
        if(it != s_Objects.end()) {
            s_Objects.erase(it);
        }

        object->m_uuid = uuid;
        s_Objects[uuid] = object;
    }
}
/*!
    Replaces current cloned \a uuid of the \a object with the new one.
    \note This is a service function. Developers shouldn't call it manually.
*/
void ObjectSystem::replaceClonedUUID(Object *object, uint32_t uuid) {
    PROFILE_FUNCTION();
    if(object) {
        object->m_cloned = uuid;
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
    Returns object with \a uuid.
    If the object doesn't exist in the hierarchy this method returns nullptr.
*/
Object *ObjectSystem::findObject(uint32_t uuid) {
    auto it = s_Objects.find(uuid);
    if(it != s_Objects.end()) {
        return it->second;
    }
    return nullptr;
}
/*!
    Adds an \a object to main pull of objects in ObjectSystem
*/
void ObjectSystem::addObject(Object *object) {
    PROFILE_FUNCTION();
    m_objectList.push_back(object);
}
/*!
    \internal
    Removes an \a object from operation lists.
    Also removes from global search dictionary.
*/
void ObjectSystem::removeObject(Object *object) {
    PROFILE_FUNCTION();

    unregisterObject(object);

    m_objectToRemove.push_back(object);
}
/*!
    \internal
    Removes an \a object from global search dictionary.
*/
void ObjectSystem::unregisterObject(Object *object) {
    auto it = s_Objects.find(object->uuid());
    if(it != s_Objects.end()) {
        s_Objects.erase(it);
    }
}
/*!
    Returns a list of objects with specified \a type.
    \warning This is very small function!
*/
Object::ObjectList ObjectSystem::getAllObjectsByType(const std::string &type) const {
    Object::ObjectList result;
    for(auto it : m_objectList) {
        auto ret = std::find(m_objectToRemove.begin(), m_objectToRemove.end(), it);
        if(ret == m_objectToRemove.end() && it->typeName() == type) {
            result.push_back(it);
        }
    }
    return result;
}
