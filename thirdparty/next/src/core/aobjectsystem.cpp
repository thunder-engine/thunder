#include "core/aobjectsystem.h"

#include "core/aobject.h"
#include "core/auri.h"
#include "core/abson.h"
#include "core/ajson.h"

class AObjectSystemPrivate {
public:
    AObjectSystemPrivate() :
        m_Exit(false) {
    }

    /// Container for registered callbacks.
    AObjectSystem::FactoryMap   m_Factories;
    AObjectSystem::GroupMap     m_Groups;

    bool                        m_Exit;

    uint32_t                    m_NextID;

    static AObjectSystem       *s_Instance;
};

AObjectSystem *AObjectSystemPrivate::s_Instance    = nullptr;

AObjectSystem::AObjectSystem(const string &name) :
        p_ptr(new AObjectSystemPrivate()) {
    PROFILE_FUNCTION()
    if(AObjectSystemPrivate::s_Instance != nullptr) {
        throw "There should be only one AObjectSystem object";
    }
    AObjectSystemPrivate::s_Instance   = this;
    setName(name);
    p_ptr->m_NextID = 1000;
}

AObjectSystem::~AObjectSystem() {
    PROFILE_FUNCTION()
    factoryClear();
    AObjectSystemPrivate::s_Instance   = nullptr;
}

int32_t AObjectSystem::exec() {
    PROFILE_FUNCTION()
    while(!p_ptr->m_Exit) {

    }
    return 0;
}

AObjectSystem *AObjectSystem::instance() {
    PROFILE_FUNCTION()
    return AObjectSystemPrivate::s_Instance;
}

AObject *AObjectSystem::objectCreate(const string &uri, const string &name, AObject *parent) {
    PROFILE_FUNCTION()
    AObject *object = nullptr;

    AObjectSystem *inst = instance();
    FactoryMap::iterator it = inst->p_ptr->m_Factories.find(uri);
    if(it == inst->p_ptr->m_Factories.end()) {
        it  = inst->p_ptr->m_Factories.find(inst->p_ptr->m_Groups[uri]);
    }
    if(it != inst->p_ptr->m_Factories.end()) {
        object = (*it).second->createInstance();
        object->setName(name);
        object->setParent(parent);
        object->setUUID(inst->nextID());
    }
    return object;
}

void AObjectSystem::factoryAdd(const string &name, const string &uri, const AMetaObject *meta) {
    PROFILE_FUNCTION()
    p_ptr->m_Groups[name]    = uri;
    p_ptr->m_Factories[uri]  = meta;
}

void AObjectSystem::factoryRemove(const string &name, const string &uri) {
    PROFILE_FUNCTION()
    p_ptr->m_Groups.erase(name);
    p_ptr->m_Factories.erase(uri);
}

void AObjectSystem::factoryClear() {
    PROFILE_FUNCTION()
    p_ptr->m_Factories.clear();
}

AObjectSystem::GroupMap AObjectSystem::factories() const {
    PROFILE_FUNCTION()
    return p_ptr->m_Groups;
}

typedef list<const AObject *> ObjectArray;

void enumObjects(const AObject *object, ObjectArray &list) {
    PROFILE_FUNCTION()
    list.push_back(object);
    for(const auto &it : object->getChildren()) {
        enumObjects(it, list);
    }
}

AVariant AObjectSystem::toVariant(const AObject *object) {
    PROFILE_FUNCTION()
    AVariantList result;

    ObjectArray array;
    enumObjects(object, array);

    for(auto it : array) {
        // Save Object
        int uuid    = int(it->uuid());

        AVariantList o;
        o.push_back(uuid);
        AObject *parent = it->parent();
        o.push_back(int((parent) ? parent->uuid() : 0));
        o.push_back(it->typeName());
        o.push_back(it->name());
        o.push_back(it->isEnable());

        // Save base properties
        AVariantMap properties;
        const AMetaObject *meta = it->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            AMetaProperty p = meta->property(i);
            if(p.isValid()) {
                AVariant v  = p.read(it);
                if(v.userType() < AMetaType::USERTYPE) {
                    properties[p.name()] = v;
                }
            }
        }

        // Save links
        AVariantList links;
        for(const auto &l : it->getReceivers()) {
            AVariantList link;

            AObject *sender   = l.sender;

            link.push_back(int(sender->uuid()));
            AMetaMethod method  = sender->metaObject()->method(l.signal);
            link.push_back(AVariant(char(method.type() + 0x30) + method.signature()));

            link.push_back(uuid);
            method      = it->metaObject()->method(l.method);
            link.push_back(AVariant(char(method.type() + 0x30) + method.signature()));

            links.push_back(link);
        }
        o.push_back(properties);
        o.push_back(it->saveUserData());
        o.push_back(links);

        result.push_back(o);
    }

    return result;
}

AObject *AObjectSystem::toObject(const AVariant &variant) {
    PROFILE_FUNCTION()
    AObject *result = nullptr;

    // Create all declared objects
    AVariantList objects    = variant.value<AVariantList>();
    ObjectMap array;
    for(auto it : objects) {
        AVariantList o  = it.value<AVariantList>();
        if(o.size() >= 5) {
            auto i      = o.begin();
            string uuid = (*i).toString();
            i++;
            AObject *parent = nullptr;
            auto a  = array.find((*i).toString());
            if(a != array.end()) {
                parent  = (*a).second;
            }
            i++;
            string type = (*i).toString();
            i++;
            string name = (*i).toString();
            i++;
            bool enable = (*i).toBool();
            i++;

            AObject *object = objectCreate(type, name, parent);
            if(object) {
                if(!object->parent()) {
                    result  = object;
                }
                object->setEnable(enable);
                array[uuid] = object;
                // Load base properties
                for(const auto &it : (*i).toMap()) {
                    AVariant v  = it.second;
                    if(v.type() < AMetaType::USERTYPE) {
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
        AVariantList o  = it.value<AVariantList>();
        AVariantList list   = o.back().value<AVariantList>();
        for(const auto &link : list) {
            AVariantList l  = link.value<AVariantList>();
            AObject *sender     = nullptr;
            AObject *receiver   = nullptr;
            if(l.size() == 4) {
                auto i  = l.begin();
                auto s = array.find((*i).toString());
                if(s != array.end()) {
                    sender  = (*s).second;
                }
                i++;

                string signal = (*i).toString();
                i++;

                s = array.find((*i).toString());
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

uint32_t AObjectSystem::nextID() {
    PROFILE_FUNCTION()
    return p_ptr->m_NextID++;
}
