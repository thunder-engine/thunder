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
        m_object(nullptr),
        m_start(nullptr),
        m_update(nullptr),
        m_metaObject(nullptr) {
    PROFILE_FUNCTION();
}

AngelBehaviour::~AngelBehaviour() {
    PROFILE_FUNCTION();
    if(m_object) {
        m_object->Release();
        m_object = nullptr;
    }
    notifyObservers();
}

std::string AngelBehaviour::script() const {
    PROFILE_FUNCTION();
    return m_script;
}

void AngelBehaviour::setScript(const std::string value) {
    PROFILE_FUNCTION();
    if(value != m_script && value != "AngelBehaviour") {
        m_script = value;

        createObject();
    }
}

void AngelBehaviour::createObject() {
    PROFILE_FUNCTION();
    if(m_object) {
        m_object->Release();
        m_object = nullptr;
    }

    AngelSystem *ptr = static_cast<AngelSystem *>(system());
    asITypeInfo *type = ptr->module()->GetTypeInfoByDecl(m_script.c_str());
    if(type) {
        int result = ptr->context()->PushState();
        std::string stream = m_script + " @+" + m_script + "()";
        asIScriptFunction *func = type->GetFactoryByDecl(stream.c_str());
        asIScriptObject **obj = static_cast<asIScriptObject **>(ptr->execute(nullptr, func));
        if(obj != nullptr) {
            asIScriptObject *object = *obj;
            if(object) {
                setScriptObject(object);
            } else {
                Log(Log::ERR) << __FUNCTION__ << "Can't create an object" << m_script;
            }
            if(result == 0) {
                ptr->context()->PopState();
                if(object) {
                    object->AddRef();
                }
            }
        } else {
            Log(Log::ERR) << __FUNCTION__ << "Systen returned NULL during execution" << m_script;
        }
    }
}

asIScriptObject *AngelBehaviour::scriptObject() const {
    PROFILE_FUNCTION();
    if(m_object) {
        m_object->AddRef();
    }
    return m_object;
}

void AngelBehaviour::setScriptObject(asIScriptObject *object) {
    PROFILE_FUNCTION();
    m_object = object;
    if(m_object) {
        m_object->AddRef();
        m_object->SetUserData(this);
        asITypeInfo *info = m_object->GetObjectType();
        if(info) {
            if(object->GetPropertyCount() > 0) {
                void *ptr = this;
                memcpy(object->GetAddressOfProperty(0), &ptr, sizeof(void *));
            }

            if(m_script.empty()) {
                m_script = info->GetName();
            }
            m_start = info->GetMethodByDecl("void start()");
            m_update = info->GetMethodByDecl("void update()");

            updateMeta();
        }
        notifyObservers();
    }
}

void AngelBehaviour::updateMeta() {
    PROFILE_FUNCTION();
    delete m_metaObject;
    m_propertyTable.clear();
    m_methodTable.clear();
    m_propertyAdresses.clear();

    const MetaObject *super = AngelBehaviour::metaClass();
    asIScriptEngine *engine = m_object->GetEngine();

    asITypeInfo *info = m_object->GetObjectType();
    uint32_t count = info->GetPropertyCount();
    for(uint32_t i = 0; i <= count; i++) {
        if(i == count) {
            m_propertyTable.push_back({nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr});
        } else {
            const char *name;
            int typeId;
            bool isPrivate;
            bool isProtected;
            info->GetProperty(i, &name, &typeId, &isPrivate, &isProtected);
            if(!isPrivate && !isProtected) {
                uint32_t metaType = 0;
                PropertyFields propertyFields;
                propertyFields.isScript = false;
                propertyFields.isObject = false;
                propertyFields.object = nullptr;
                propertyFields.address = nullptr;
                if(typeId > asTYPEID_DOUBLE) {
                    asITypeInfo *type = engine->GetTypeInfoById(typeId);
                    if(type) {
                        metaType = MetaType::type(type->GetName());
                        if(type->GetFlags() & asOBJ_REF) {
                            metaType++;
                        }

                        auto factory = System::metaFactory(type->GetName());
                        if(factory) {
                            propertyFields.isObject = true;
                        }

                        if(type->GetFlags() & asOBJ_SCRIPT_OBJECT) {
                            propertyFields.isScript = true;
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
                    propertyFields.address = m_object->GetAddressOfProperty(i);
                    m_propertyAdresses[name] = propertyFields;
                    m_propertyTable.push_back({name, table, nullptr, nullptr, nullptr, nullptr, nullptr,
                                               &Reader<decltype(&AngelBehaviour::readProperty), &AngelBehaviour::readProperty>::read,
                                               &Writer<decltype(&AngelBehaviour::writeProperty), &AngelBehaviour::writeProperty>::write});
                }
            }
        }
    }

    count = info->GetMethodCount();
    for(uint32_t m = 0; m <= count; m++) {
        if(m == count) {
            m_methodTable.push_back({MetaMethod::Method, nullptr, nullptr, nullptr, 0, nullptr});
        } else {
            asIScriptFunction *method = info->GetMethodByIndex(m);
            if(method) {
                std::string name(method->GetName());
                if(name.size() > 2 && name[0] == 'o' && name[1] == 'n') { // this is a slot
                    m_methodTable.push_back(A_SLOTEX(AngelBehaviour::scriptSlot, method->GetName()));
                }
            }
        }
    }

    m_metaObject = new MetaObject(m_script.c_str(),
                                   super, &AngelBehaviour::construct,
                                   &m_methodTable[0], &m_propertyTable[0], nullptr);
}

asIScriptFunction *AngelBehaviour::scriptStart() const {
    PROFILE_FUNCTION();
    return m_start;
}

asIScriptFunction *AngelBehaviour::scriptUpdate() const {
    PROFILE_FUNCTION();
    return m_update;
}

const MetaObject *AngelBehaviour::metaObject() const {
    PROFILE_FUNCTION();
    if(m_metaObject) {
        return m_metaObject;
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
    VariantMap result;
    for(auto it : m_propertyTable) {
        if(it.name) {
            Variant value = MetaProperty(&it).read(this);

            std::string typeName = it.type->name;
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
                result[it.name] = value;
            }
        }
    }
    return result;
}

void AngelBehaviour::loadUserData(const VariantMap &data) {
    PROFILE_FUNCTION();
    for(auto it : m_propertyTable) {
        if(it.name) {
            auto property = data.find(it.name);
            if(property != data.end()) {
                std::string typeName = it.type->name;
                if(typeName.back() == '*') {
                    typeName = typeName.substr(0, typeName.size() - 2);
                }
                auto factory = System::metaFactory(typeName);
                if(factory) {
                    Object *object = nullptr;
                    if(factory->first->canCastTo(RESOURCE)) {
                        Resource *resource = Engine::loadResource<Resource>(property->second.toString());
                        if(resource) {
                            resource->incRef();
                        }
                        object = resource;
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

void AngelBehaviour::setType(const std::string &type) {
    PROFILE_FUNCTION();
    setScript(type);
}

void AngelBehaviour::scriptSlot() {
    // Method placeholder for the all incoming signals
}

void AngelBehaviour::onReferenceDestroyed() {
    Object *object = sender();
    for(auto &it : m_propertyAdresses) {
        if(it.second.object == object) {
            void *null = nullptr;
            memcpy(it.second.address, &null, sizeof(null));
        }
    }
}

Variant AngelBehaviour::readProperty(const MetaProperty &property) const {
    PROFILE_FUNCTION();
    auto it = m_propertyAdresses.find(property.name());
    if(it != m_propertyAdresses.end()) {
        if(it->second.isScript) {
            if(it->second.address) {
                AngelBehaviour *behaviour = nullptr;
                asIScriptObject *object = *(reinterpret_cast<asIScriptObject **>(it->second.address));
                if(object) {
                    behaviour = reinterpret_cast<AngelBehaviour *>(object->GetUserData());
                }
                return Variant(MetaType::type(property.table()->type->name), &behaviour);
            }
        } else {
            return Variant(MetaType::type(property.table()->type->name), it->second.address);
        }
    }
    return Variant();
}

void AngelBehaviour::writeProperty(const MetaProperty &property, const Variant value) {
    PROFILE_FUNCTION();
    auto it = m_propertyAdresses.find(property.name());
    if(it != m_propertyAdresses.end()) {
        if(it->second.isScript) {
            AngelBehaviour *behaviour = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<AngelBehaviour **>(value.data()));
            it->second.object = behaviour;
            if(behaviour) {
                asIScriptObject *script = *(reinterpret_cast<asIScriptObject **>(it->second.address));
                if(script) {
                    script->Release();
                }
                script = behaviour->scriptObject();
                behaviour->subscribe(this, it->second.address);
                memcpy(it->second.address, &script, sizeof(script));
                return;
            }
        }
        if(it->second.isObject) {
            Object *object = nullptr;
            if(value.type() == MetaType::INTEGER) {
                uint32_t uuid = value.toInt();
                if(uuid) {
                    object = Engine::findObject(uuid, Engine::findRoot(this));
                }
            } else {
                object = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
            }
            if(it->second.object != object) {
                disconnect(it->second.object, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
                it->second.object = object;
                connect(it->second.object, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
            }
        }

        memcpy(it->second.address, value.data(), MetaType(property.table()->type).size());
    }
}

void AngelBehaviour::methodCallEvent(MethodCallEvent *event) {
    PROFILE_FUNCTION();
    if(event && m_metaObject) {
        MetaMethod method = m_metaObject->method(event->method());
        if(method.isValid() && m_object) {
            asITypeInfo *info = m_object->GetObjectType();
            if(info) {
                std::string signature("void ");
                signature += method.signature();
                asIScriptFunction *func = info->GetMethodByDecl(signature.c_str());
                if(func) {
                    AngelSystem *ptr = static_cast<AngelSystem *>(system());
                    ptr->execute(m_object, func);
                    return;
                }
            }
        }
        Object::methodCallEvent(event);
    }
}

void AngelBehaviour::subscribe(AngelBehaviour *observer, void *ptr) {
    m_obsevers.push_back(std::pair<AngelBehaviour *, void *>(observer, ptr));
}

void AngelBehaviour::notifyObservers() {
    for(auto &it : m_obsevers) {
        if(m_object) {
            m_object->AddRef();
        }
        memcpy(it.second, &m_object, sizeof(m_object));
    }
    m_obsevers.clear();
}
