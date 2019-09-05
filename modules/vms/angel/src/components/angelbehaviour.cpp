#include "components/angelbehaviour.h"

#include "angelsystem.h"

#include <cstring>

#include <log.h>

#include <angelscript.h>

#include <analytics/profiler.h>

AngelBehaviour::AngelBehaviour() :
        m_pObject(nullptr),
        m_pStart(nullptr),
        m_pUpdate(nullptr),
        m_pMetaObject(nullptr) {
    PROFILER_MARKER;
}

AngelBehaviour::~AngelBehaviour() {
    PROFILER_MARKER;
    if(m_pObject) {
        m_pObject->Release();
    }
}

string AngelBehaviour::script() const {
    PROFILER_MARKER;
    return m_Script;
}

void AngelBehaviour::setScript(const string &value) {
    PROFILER_MARKER;
    if(value != m_Script) {
        if(m_pObject) {
            m_pObject->Release();
            m_pObject = nullptr;
        }
        m_Script = value;

        AngelSystem *ptr = static_cast<AngelSystem *>(system());
        asITypeInfo *type = ptr->module()->GetTypeInfoByDecl(value.c_str());
        if(type) {
            string stream = value + " @+" + value + "()";
            ptr->execute(nullptr, type->GetFactoryByDecl(stream.c_str()));

            asIScriptObject **obj = static_cast<asIScriptObject **>(ptr->context()->GetAddressOfReturnValue());
            if(obj == nullptr) {
                return;
            }
            asIScriptObject *object = *obj;
            if(object) {
                object->AddRef();

                setScriptObject(object);
            } else {
                Log(Log::ERR) << __FUNCTION__ << "Can't create an object" << value.c_str();
            }
        }
    }
}

asIScriptObject *AngelBehaviour::scriptObject() const {
    return m_pObject;
}

void AngelBehaviour::setScriptObject(asIScriptObject *object) {
    PROFILER_MARKER;
    m_pObject   = object;
    if(m_pObject) {
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

            if(m_pMetaObject) {
                delete m_pMetaObject;
            }
            const MetaObject *super = AngelBehaviour::metaClass();

            asIScriptEngine *engine = m_pObject->GetEngine();

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
                            void *ptr = object->GetAddressOfProperty(i);
                            m_Table.push_back({name, table, nullptr, nullptr, nullptr, nullptr, nullptr, ptr});
                        }
                    }
                }
            }

            m_pMetaObject = new MetaObject(m_Script.c_str(),
                                           super,
                                           &AngelBehaviour::construct,
                                           nullptr,
                                           &m_Table[0]);
        }
    }
}

asIScriptFunction *AngelBehaviour::scriptStart() const {
    PROFILER_MARKER;
    return m_pStart;
}

asIScriptFunction *AngelBehaviour::scriptUpdate() const {
    PROFILER_MARKER;
    return m_pUpdate;
}

const MetaObject *AngelBehaviour::metaObject() const {
    PROFILER_MARKER;
    if(m_pMetaObject) {
        return m_pMetaObject;
    }
    return AngelBehaviour::metaClass();
}

VariantList AngelBehaviour::saveData() const {
    PROFILER_MARKER;
    return serializeData(AngelBehaviour::metaClass());
}

void AngelBehaviour::loadData(const VariantList &data) {
    PROFILER_MARKER;
    Object::loadData(data);
}

VariantMap AngelBehaviour::saveUserData() const {
    PROFILER_MARKER;
    VariantMap result = NativeBehaviour::saveUserData();
    for(auto it : m_Table) {
        if(it.ptr != nullptr) {
            Variant value = MetaProperty(&it).read(this);
            if(value.type() == MetaType::USERTYPE && (value.userType() == MetaType::type<Actor *>())) {
                result[it.name] = Engine::reference(*(reinterpret_cast<Object **>(value.data())));
            } else {
                result[it.name] = MetaProperty(&it).read(this);
            }
        }
    }
    return result;
}

void AngelBehaviour::loadUserData(const VariantMap &data) {
    PROFILER_MARKER;
    Object::loadUserData(data);
    for(auto it : m_Table) {
        if(it.ptr != nullptr) {
            auto property = data.find(it.name);
            if(property != data.end()) {
                uint32_t type = MetaType::type(MetaType(it.type).name());
                if(type == MetaType::type<Actor *>()) {
                    Object *actor = Engine::loadResource<Object>(property->second.toString());
                    MetaProperty(&it).write(this, Variant(type, &actor));
                } else {
                    MetaProperty(&it).write(this, property->second);
                }
            }
        }
    }
}
