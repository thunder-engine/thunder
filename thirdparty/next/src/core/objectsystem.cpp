#include "core/objectsystem.h"

#include "core/object.h"
#include "core/uri.h"
#include "core/bson.h"
#include "core/json.h"

#include <random>

static random_device rd;
static mt19937 mt(rd());
static uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

class ObjectSystemPrivate {
public:
    ObjectSystemPrivate() {

    }

    /// Container for registered callbacks.
    ObjectSystem::FactoryMap    m_Factories;
    ObjectSystem::GroupMap      m_Groups;

    static ObjectSystem        *s_Instance;
};

ObjectSystem *ObjectSystemPrivate::s_Instance    = nullptr;
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
    Constructs ObjectSystem with \a name.
    \note There can be only one instance of Object System per application. (semi singleton)
*/
ObjectSystem::ObjectSystem(const string &name) :
        p_ptr(new ObjectSystemPrivate()) {
    PROFILE_FUNCTION()
    if(ObjectSystemPrivate::s_Instance != nullptr) {
        throw "There should be only one ObjectSystem object";
    }
    ObjectSystemPrivate::s_Instance   = this;
    setName(name);
}

ObjectSystem::~ObjectSystem() {
    PROFILE_FUNCTION()
    factoryClear();
    ObjectSystemPrivate::s_Instance   = nullptr;
}

/*!
    Returns a singe instance of ObjectSystem.
    \note Can return nullptr in case of ObjectSystem isn't initialized yet.
*/
ObjectSystem *ObjectSystem::instance() {
    PROFILE_FUNCTION()
    return ObjectSystemPrivate::s_Instance;
}
/*!
    Returns new instance of type represented in \a uri and \a name as child of \a parent object.
    \note Class represented as uri should be registered first via factoryAdd()

    \sa factoryAdd(), factoryRemove()
*/
Object *ObjectSystem::objectCreate(const string &uri, const string &name, Object *parent) {
    PROFILE_FUNCTION()
    Object *object  = nullptr;

    ObjectSystem *inst = instance();
    FactoryMap::iterator it = inst->p_ptr->m_Factories.find(uri);
    if(it == inst->p_ptr->m_Factories.end()) {
        it  = inst->p_ptr->m_Factories.find(inst->p_ptr->m_Groups[uri]);
    }
    if(it != inst->p_ptr->m_Factories.end()) {
        object = (*it).second->createInstance();
        object->setName(name);
        object->setParent(parent);
        object->setUUID(generateUID());
    }
    return object;
}

void ObjectSystem::processObject(Object *object) {
    object->processEvents();
}

void ObjectSystem::factoryAdd(const string &name, const string &uri, const MetaObject *meta) {
    PROFILE_FUNCTION()
    p_ptr->m_Groups[name]   = uri;
    p_ptr->m_Factories[uri] = meta;
}

void ObjectSystem::factoryRemove(const string &name, const string &uri) {
    PROFILE_FUNCTION()
    p_ptr->m_Groups.erase(name);
    p_ptr->m_Factories.erase(uri);
}
/*!
    Removes all factories from the system.
*/
void ObjectSystem::factoryClear() {
    PROFILE_FUNCTION()
    p_ptr->m_Factories.clear();
}
/*!
    Returns all registered classes.
*/
ObjectSystem::GroupMap ObjectSystem::factories() const {
    PROFILE_FUNCTION()
    return p_ptr->m_Groups;
}

typedef list<const Object *>    ObjectArray;

void enumObjects(const Object *object, ObjectArray &list) {
    PROFILE_FUNCTION()
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
    PROFILE_FUNCTION()
    VariantList result;

    ObjectArray array;
    enumObjects(object, array);

    for(auto it : array) {
        // Save Object
        int uuid    = int(it->uuid());

        VariantList o;
        o.push_back(uuid);
        Object *parent = it->parent();
        o.push_back(int((parent) ? parent->uuid() : 0));
        o.push_back(it->typeName());
        o.push_back(it->name());

        // Save base properties
        VariantMap properties;
        const MetaObject *meta = it->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            MetaProperty p = meta->property(i);
            if(p.isValid()) {
                Variant v  = p.read(it);
                if(v.userType() < MetaType::USERTYPE) {
                    properties[p.name()] = v;
                }
            }
        }

        // Save links
        VariantList links;
        for(const auto &l : it->getReceivers()) {
            VariantList link;

            Object *sender  = l.sender;

            link.push_back(int(sender->uuid()));
            MetaMethod method  = sender->metaObject()->method(l.signal);
            link.push_back(Variant(char(method.type() + 0x30) + method.signature()));

            link.push_back(uuid);
            method      = it->metaObject()->method(l.method);
            link.push_back(Variant(char(method.type() + 0x30) + method.signature()));

            links.push_back(link);
        }
        o.push_back(properties);
        o.push_back(links);
        o.push_back(it->saveUserData());

        result.push_back(o);
    }

    return result;
}
/*!
    Returns object deserialized from \a variant based representation.
    The Variant representation can be loaded from BSON or JSON formats or retrieved from memory.
    Deserialization will try to restore objects hierarchy, its properties and connections.
*/
Object *ObjectSystem::toObject(const Variant &variant) {
    PROFILE_FUNCTION()
    Object *result  = nullptr;

    typedef unordered_map<uint32_t, Object *> ObjectMap;
    ObjectMap array;

    // Create all declared objects
    VariantList objects    = variant.value<VariantList>();
    for(auto it : objects) {
        VariantList o  = it.value<VariantList>();
        if(o.size() >= 5) {
            auto i          = o.begin();
            uint32_t uuid   = (*i).toInt();
            i++;
            Object *parent  = nullptr;
            auto a  = array.find((*i).toInt());
            if(a != array.end()) {
                parent  = (*a).second;
            }
            i++;
            string type = (*i).toString();
            i++;
            string name = (*i).toString();
            i++;

            Object *object  = objectCreate(type, name, parent);
            if(object) {
                object->setUUID(uuid);
                if(!object->parent()) {
                    result  = object;
                }
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
            }
        }
    }

    return result;
}
/*!
    Returns the new unique ID based on random number generator.
*/
uint32_t ObjectSystem::generateUID() {
    return dist(mt);
}
