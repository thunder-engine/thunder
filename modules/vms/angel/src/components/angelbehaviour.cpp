#include "components/angelbehaviour.h"

#include <resources/prefab.h>
#include <metamethod.h>

#include <cstring>

#include <log.h>

#include <angelscript.h>

#include "angelsystem.h"

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
    /// \todo Need to release references for the properties
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
    PROFILE_FUNCTION();
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
    PROFILE_FUNCTION();
    if(m_pObject) {
        m_pObject->AddRef();
    }
    return m_pObject;
}

void AngelBehaviour::setScriptObject(asIScriptObject *object) {
    PROFILE_FUNCTION();
    m_pObject = object;
    if(m_pObject) {
        m_pObject->AddRef();
        m_pObject->SetUserData(this);
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
        notifyObservers();
    }
}

void AngelBehaviour::updateMeta() {
    PROFILE_FUNCTION();
    delete m_pMetaObject;
    m_PropertyTable.clear();
    m_MethodTable.clear();
    m_PropertyAdresses.clear();

    const MetaObject *super = AngelBehaviour::metaClass();
    asIScriptEngine *engine = m_pObject->GetEngine();

    asITypeInfo *info = m_pObject->GetObjectType();
    uint32_t count = info->GetPropertyCount();
    for(uint32_t i = 0; i <= count; i++) {
        if(i == count) {
            m_PropertyTable.push_back({nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr});
        } else {
            const char *name;
            int typeId;
            bool isPrivate;
            bool isProtected;
            info->GetProperty(i, &name, &typeId, &isPrivate, &isProtected);
            if(!isPrivate && !isProtected) {
                uint32_t metaType = 0;
                bool isScriptType = false;
                if(typeId > asTYPEID_DOUBLE) {
                    asITypeInfo *type = engine->GetTypeInfoById(typeId);
                    if(type) {
                        metaType = MetaType::type(type->GetName());
                        if(type->GetFlags() & asOBJ_REF) {
                            metaType++;
                        }

                        if(type->GetFlags() & asOBJ_SCRIPT_OBJECT) {
                            isScriptType = true;
                        }
                    }
                } else {
                    switch(typeId) {
                    case asTYPEID_VOID:   metaType = MetaType::INVALID; break;
                    case asTYPEID_BOOL:   metaType = MetaType::BOOLEAN; break;
                    case asTYPEID_INT8:
                    case asTYPEID_INT16:
                    case asTYPEID_INT32:
                    case asTYPEID_INT64:
                    case asTYPEID_UINT8:
                    case asTYPEID_UINT16:
                    case asTYPEID_UINT32:
                    case asTYPEID_UINT64: metaType = MetaType::INTEGER; break;
                    case asTYPEID_FLOAT:
                    case asTYPEID_DOUBLE: metaType = MetaType::FLOAT; break;
                    default: break;
                    }
                }
                MetaType::Table *table = MetaType::table(metaType);
                if(table) {
                    const char *annotation = (isScriptType) ? "@" : nullptr;
                    m_PropertyAdresses[name] = m_pObject->GetAddressOfProperty(i);
                    m_PropertyTable.push_back({name, table, annotation, nullptr, nullptr, nullptr, nullptr,
                                               &Reader<decltype(&AngelBehaviour::readProperty), &AngelBehaviour::readProperty>::read,
                                               &Writer<decltype(&AngelBehaviour::writeProperty), &AngelBehaviour::writeProperty>::write});
                }
            }
        }
    }

    count = info->GetMethodCount();
    for(uint32_t m = 0; m <= count; m++) {
        if(m == count) {
            m_MethodTable.push_back({MetaMethod::Method, nullptr, nullptr, nullptr, 0, nullptr});
        } else {
            asIScriptFunction *method = info->GetMethodByIndex(m);
            if(method) {
                string name(method->GetName());
                if(name.size() > 2 && name[0] == 'o' && name[1] == 'n') { // this is a slot
                    m_MethodTable.push_back(A_SLOTEX(AngelBehaviour::scriptSlot, method->GetName()));
                }
            }
        }
    }

    m_pMetaObject = new MetaObject(m_Script.c_str(),
                                   super, &AngelBehaviour::construct,
                                   &m_MethodTable[0], &m_PropertyTable[0], nullptr);
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
    for(auto it : m_PropertyTable) {
        if(it.name) {
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
    for(auto it : m_PropertyTable) {
        if(it.name) {
            auto property = data.find(it.name);
            if(property != data.end()) {
                string typeName = it.type->name;
                if(typeName.back() == '*') {
                    typeName = typeName.substr(0, typeName.size() - 2);
                }
                auto factory = System::metaFactory(typeName);
                if(factory) {
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
                        uint32_t type = MetaType::type(MetaType(it.type).name());
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

void AngelBehaviour::scriptSlot() {
    // Method placeholder for the all incoming signals
}

void AngelBehaviour::onReferenceDestroyed() {

}

Variant AngelBehaviour::readProperty(const MetaProperty &property) const {
    PROFILE_FUNCTION();
    auto it = m_PropertyAdresses.find(property.name());
    if(it != m_PropertyAdresses.end()) {
        const char *annotation = property.table()->annotation;
        if(annotation && property.table()->annotation[0] == '@') {
            if(it->second) {
                AngelBehaviour *behaviour = nullptr;
                asIScriptObject *object = *(reinterpret_cast<asIScriptObject **>(it->second));
                if(object) {
                    behaviour = reinterpret_cast<AngelBehaviour *>(object->GetUserData());
                }
                return Variant(MetaType::type(property.table()->type->name), &behaviour);
            }
        } else {
            return Variant(MetaType::type(property.table()->type->name), it->second);
        }
    }
    return Variant();
}

void AngelBehaviour::writeProperty(const MetaProperty &property, const Variant &value) {
    PROFILE_FUNCTION();
    auto it = m_PropertyAdresses.find(property.name());
    if(it != m_PropertyAdresses.end()) {
        const char *annotation = property.table()->annotation;
        if(annotation && annotation[0] == '@') {
            AngelBehaviour *behaviour = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<AngelBehaviour **>(value.data()));
            if(behaviour) {
                asIScriptObject *script = *(reinterpret_cast<asIScriptObject **>(it->second));
                if(script) {
                    script->Release();
                }
                script = behaviour->scriptObject();
                if(script == nullptr) {
                    behaviour->subscribe(this, it->second);
                }
                memcpy(it->second, &script, sizeof(script));
                return;
            }
        }
        memcpy(it->second, value.data(), MetaType(property.table()->type).size());
    }
}

void AngelBehaviour::methodCallEvent(MethodCallEvent *event) {
    PROFILE_FUNCTION();
    if(event) {
        MetaMethod method = m_pMetaObject->method(event->method());
        if(method.isValid()) {
            asITypeInfo *info = m_pObject->GetObjectType();
            if(info) {
                string signature("void ");
                signature += method.signature();
                asIScriptFunction *func = info->GetMethodByDecl(signature.c_str());
                if(func) {
                    AngelSystem *ptr = static_cast<AngelSystem *>(system());
                    ptr->execute(m_pObject, func);
                }
            }
        }
    }
}

void AngelBehaviour::subscribe(AngelBehaviour *observer, void *ptr) {
    m_Obsevers.push_back(pair<AngelBehaviour *, void *>(observer, ptr));
}

void AngelBehaviour::notifyObservers() {
    for(auto &it : m_Obsevers) {
        m_pObject->AddRef();
        memcpy(it.second, &m_pObject, sizeof(m_pObject));
    }
    m_Obsevers.clear();
}
