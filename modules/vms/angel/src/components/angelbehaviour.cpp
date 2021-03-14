#include "components/angelbehaviour.h"

#include "resources/prefab.h"

#include "angelsystem.h"

#include <cstring>

#include <log.h>

#include <angelscript.h>

#define RESOURCE "Resource"
#define GENERAL "General"

AngelBehaviour::AngelBehaviour() :
        m_pObject(nullptr),
        m_pStart(nullptr),
        m_pUpdate(nullptr),
        m_pMetaObject(nullptr) {
    PROFILE_FUNCTION();
}

AngelBehaviour::~AngelBehaviour() {
    PROFILE_FUNCTION();
    if(m_pObject) {
        m_pObject->Release();
    }
}

string AngelBehaviour::script() const {
    PROFILE_FUNCTION();
    return m_Script;
}

void AngelBehaviour::setScript(const string &value) {
    PROFILE_FUNCTION();
    if(value != m_Script) {
        m_Script = value;

        createObject();
    }
}

void AngelBehaviour::createObject() {
    if(m_pObject) {
        m_pObject->Release();
        m_pObject = nullptr;
    }

    AngelSystem *ptr = static_cast<AngelSystem *>(system());
    asITypeInfo *type = ptr->module()->GetTypeInfoByDecl(m_Script.c_str());
    if(type) {
        int result = ptr->context()->PushState();
        string stream = m_Script + " @+" + m_Script + "()";
        asIScriptFunction *func = type->GetFactoryByDecl(stream.c_str());
        asIScriptObject **obj = static_cast<asIScriptObject **>(ptr->execute(nullptr, func));
        if(obj == nullptr) {
            return;
        }
        asIScriptObject *object = *obj;
        if(object) {
            setScriptObject(object);
        } else {
            Log(Log::ERR) << __FUNCTION__ << "Can't create an object" << m_Script.c_str();
        }
        if(result == 0) {
            ptr->context()->PopState();
            if(object) {
                object->AddRef();
            }
        }
    }
}

asIScriptObject *AngelBehaviour::scriptObject() const {
    return m_pObject;
}

void AngelBehaviour::setScriptObject(asIScriptObject *object) {
    PROFILE_FUNCTION();
    m_pObject = object;
    if(m_pObject) {
        m_pObject->AddRef();
        asITypeInfo *info = m_pObject->GetObjectType();
        if(info) {
            if(object->GetPropertyCount() > 0) {
                void *ptr = this;
                memcpy(object->GetAddressOfProperty(0), &ptr, sizeof(void *));
            }

            if(m_Script.empty()) {
                m_Script = info->GetName();
            }
            m_pStart = info->GetMethodByDecl("void start()");
            m_pUpdate = info->GetMethodByDecl("void update()");

            updateMeta();
        }
    }
}

void AngelBehaviour::updateMeta() {
    delete m_pMetaObject;
    m_Table.clear();

    const MetaObject *super = AngelBehaviour::metaClass();
    asIScriptEngine *engine = m_pObject->GetEngine();

    asITypeInfo *info = m_pObject->GetObjectType();
    uint32_t count = info->GetPropertyCount();
    for(uint32_t i = 0; i <= count; i++) {
        if(i == count) {
            m_Table.push_back({nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr});
        } else {
            const char *name;
            int typeId;
            bool isPrivate;
            bool isProtected;
            info->GetProperty(i, &name, &typeId, &isPrivate, &isProtected);
            if(!isPrivate && !isProtected) {
                uint32_t metaType = 0;
                if(typeId > asTYPEID_DOUBLE) {
                    asITypeInfo *type = engine->GetTypeInfoById(typeId);
                    if(type) {
                        metaType = MetaType::type(type->GetName());
                        if(type->GetFlags() & asOBJ_REF) {
                            metaType++;
                        }
                    }
                } else {
                    switch(typeId) {
                    case asTYPEID_VOID:     metaType = MetaType::INVALID; break;
                    case asTYPEID_BOOL:     metaType = MetaType::BOOLEAN; break;
                    case asTYPEID_INT8:
                    case asTYPEID_INT16:
                    case asTYPEID_INT32:
                    case asTYPEID_INT64:
                    case asTYPEID_UINT8:
                    case asTYPEID_UINT16:
                    case asTYPEID_UINT32:
                    case asTYPEID_UINT64:   metaType = MetaType::INTEGER; break;
                    case asTYPEID_FLOAT:
                    case asTYPEID_DOUBLE:   metaType = MetaType::FLOAT; break;
                    default: break;
                    }
                }
                MetaType::Table *table = MetaType::table(metaType);
                if(table) {
                    void *ptr = m_pObject->GetAddressOfProperty(i);
                    m_Table.push_back({name, table, nullptr, nullptr, nullptr, nullptr, nullptr, ptr});
                }
            }
        }
    }

    m_pMetaObject = new MetaObject(m_Script.c_str(),
                                   super, &AngelBehaviour::construct,
                                   nullptr, &m_Table[0], nullptr);
}

asIScriptFunction *AngelBehaviour::scriptStart() const {
    PROFILE_FUNCTION();
    return m_pStart;
}

asIScriptFunction *AngelBehaviour::scriptUpdate() const {
    PROFILE_FUNCTION();
    return m_pUpdate;
}

const MetaObject *AngelBehaviour::metaObject() const {
    PROFILE_FUNCTION();
    if(m_pMetaObject) {
        return m_pMetaObject;
    }
    return AngelBehaviour::metaClass();
}

void AngelBehaviour::registerClassFactory(ObjectSystem *system) {
    REGISTER_META_TYPE(AngelBehaviour);
    system->factoryAdd<AngelBehaviour>(GENERAL, AngelBehaviour::metaClass());
}

void AngelBehaviour::unregisterClassFactory(ObjectSystem *system) {
    UNREGISTER_META_TYPE(AngelBehaviour);
    system->factoryRemove<AngelBehaviour>(GENERAL);
}

VariantList AngelBehaviour::saveData() const {
    PROFILE_FUNCTION();
    return serializeData(AngelBehaviour::metaClass());
}

void AngelBehaviour::loadData(const VariantList &data) {
    PROFILE_FUNCTION();
    Object::loadData(data);
}

VariantMap AngelBehaviour::saveUserData() const {
    PROFILE_FUNCTION();
    VariantMap result = NativeBehaviour::saveUserData();
    for(auto it : m_Table) {
        if(it.ptr != nullptr) {
            Variant value = MetaProperty(&it).read(this);

            string typeName = it.type->name;
            if(typeName.back() == '*') {
                typeName = typeName.substr(0, typeName.size() - 2);
            }
            auto factory = System::metaFactory(typeName);
            if(factory) {
                Object *object = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
                if(factory->first->canCastTo(RESOURCE)) {
                    result[it.name] = Engine::reference(object);
                } else {
                    uint32_t uuid = 0;
                    if(object) {
                        uuid = object->uuid();
                    }
                    result[it.name] = uuid;
                }
            } else {
                result[it.name] = MetaProperty(&it).read(this);
            }
        }
    }
    return result;
}

void AngelBehaviour::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();
    NativeBehaviour::loadUserData(data);
    for(auto it : m_Table) {
        if(it.ptr != nullptr) {
            auto property = data.find(it.name);
            if(property != data.end()) {
                string typeName = it.type->name;
                if(typeName.back() == '*') {
                    typeName = typeName.substr(0, typeName.size() - 2);
                }
                auto factory = System::metaFactory(typeName);
                if(factory) {
                    uint32_t type = MetaType::type(MetaType(it.type).name());
                    Object *object = nullptr;
                    if(factory->first->canCastTo(RESOURCE)) {
                        object = Engine::loadResource<Object>(property->second.toString());
                    } else {
                        uint32_t uuid = property->second.toInt();
                        if(uuid) {
                            object = Engine::findObject(uuid, Engine::findRoot(this));
                        }
                    }
                    if(object) {
                        MetaProperty(&it).write(this, Variant(type, &object));
                    }
                } else {
                    MetaProperty(&it).write(this, property->second);
                }
            }
        }
    }
}

void AngelBehaviour::setType(const string &type) {
    PROFILE_FUNCTION();
    setScript(type);
}
