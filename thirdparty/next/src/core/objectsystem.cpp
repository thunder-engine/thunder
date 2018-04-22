#include "core/objectsystem.h"

#include "core/object.h"
#include "core/uri.h"
#include "core/bson.h"
#include "core/json.h"

class ObjectSystemPrivate {
public:
    ObjectSystemPrivate() :
        m_Exit(false) {
    }

    /// Container for registered callbacks.
    ObjectSystem::FactoryMap    m_Factories;
    ObjectSystem::GroupMap      m_Groups;

    bool                        m_Exit;

    static ObjectSystem        *s_Instance;
};

ObjectSystem *ObjectSystemPrivate::s_Instance    = nullptr;

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

int32_t ObjectSystem::exec() {
    PROFILE_FUNCTION()
    while(!p_ptr->m_Exit) {

    }
    return 0;
}

ObjectSystem *ObjectSystem::instance() {
    PROFILE_FUNCTION()
    return ObjectSystemPrivate::s_Instance;
}

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
        object->setUUID(generateUUID(object));
    }
    return object;
}

void ObjectSystem::factoryAdd(const string &name, const string &uri, const MetaObject *meta) {
    PROFILE_FUNCTION()
    p_ptr->m_Groups[name]    = uri;
    p_ptr->m_Factories[uri]  = meta;
}

void ObjectSystem::factoryRemove(const string &name, const string &uri) {
    PROFILE_FUNCTION()
    p_ptr->m_Groups.erase(name);
    p_ptr->m_Factories.erase(uri);
}

void ObjectSystem::factoryClear() {
    PROFILE_FUNCTION()
    p_ptr->m_Factories.clear();
}

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
        o.push_back(it->saveUserData());
        o.push_back(links);

        result.push_back(o);
    }

    return result;
}

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
                // Load user data
                object->loadUserData((*i).toMap());
                i++;
            }
        }
    }
    // Restore connections
    for(auto it : objects) {
        VariantList o  = it.value<VariantList>();
        VariantList list   = o.back().value<VariantList>();
        for(const auto &link : list) {
            VariantList l  = link.value<VariantList>();
            Object *sender      = nullptr;
            Object *receiver    = nullptr;
            if(l.size() == 4) {
                auto i  = l.begin();
                auto s = array.find((*i).toInt());
                if(s != array.end()) {
                    sender  = (*s).second;
                }
                i++;

                string signal = (*i).toString();
                i++;

                s = array.find((*i).toInt());
                if(s != array.end()) {
                    receiver  = (*s).second;
                }
                i++;

                string method = (*i).toString();
                i++;

                connect(sender, signal.c_str(), receiver, method.c_str());
            }
        }
    }

    return result;
}

uint32_t ObjectSystem::generateUUID(const Object *object) {
    return hash<const void *>()(object);
}
