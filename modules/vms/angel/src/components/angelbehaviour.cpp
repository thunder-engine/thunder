#include "components/angelbehaviour.h"

#include <resources/prefab.h>
#include <metamethod.h>

#include <cstring>

#include <log.h>

#include <angelscript.h>
#include <scriptarray/scriptarray.h>

#include "angelsystem.h"

namespace {
    const char *gGeneral("General");
};

AngelBehaviour::AngelBehaviour() :
        m_object(nullptr),
        m_start(nullptr),
        m_update(nullptr) {
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

TString AngelBehaviour::script() const {
    PROFILE_FUNCTION();
    return m_script;
}

void AngelBehaviour::setScript(const TString value) {
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
    asIScriptModule *module = ptr->module();
    if(module) {
        asITypeInfo *type = module->GetTypeInfoByDecl(m_script.data());
        if(type) {
            asIScriptObject *object = static_cast<asIScriptObject *>(ptr->module()->GetEngine()->CreateScriptObject(type));
            if(object) {
                setScriptObject(object);
            } else {
                aError() << __FUNCTION__ << "Systen returned NULL during execution" << m_script;
            }
        }
    } else {
        aError() << __FUNCTION__ << "The Script Module is NULL" << m_script;
    }
}

asIScriptObject *AngelBehaviour::scriptObject() const {
    PROFILE_FUNCTION();
    return m_object;
}

void AngelBehaviour::setScriptObject(asIScriptObject *object) {
    PROFILE_FUNCTION();
    if(m_object) {
        m_object->Release();
    }
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

            if(m_script.isEmpty()) {
                m_script = info->GetName();
            }
            m_start = info->GetMethodByDecl("void start()");
            m_update = info->GetMethodByDecl("void update()");

            m_propertyFields.clear();

            asIScriptEngine *engine = m_object->GetEngine();
            uint32_t count = info->GetPropertyCount();
            for(uint32_t i = 0; i < count; i++) {
                const char *name;
                int typeId;
                bool isPrivate;
                bool isProtected;
                info->GetProperty(i, &name, &typeId, &isPrivate, &isProtected);
                if(!isPrivate && !isProtected) {
                    PropertyFields propertyFields;
                    propertyFields.address = object->GetAddressOfProperty(i);
                    if(typeId > asTYPEID_DOUBLE) {
                        asITypeInfo *type = engine->GetTypeInfoById(typeId);
                        if(type) {
                            TString typeName(type->GetName());

                            if(typeName == "array") {
                                type = type->GetSubType();
                                if(type) {
                                    typeName = type->GetName();
                                }
                                propertyFields.isArray = true;
                            }

                            auto factory = System::metaFactory(typeName);
                            if(factory) {
                                propertyFields.isObject = true;
                            }

                            if(type && type->GetFlags() & asOBJ_SCRIPT_OBJECT) {
                                propertyFields.isScript = true;
                            }
                        }
                    }

                    m_propertyFields[name] = propertyFields;
                }
            }
        }

        notifyObservers();
    }
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
    if(m_object) {
        return static_cast<AngelSystem *>(system())->getMetaObject(m_object);
    }
    return AngelBehaviour::metaClass();
}

void AngelBehaviour::registerClassFactory(ObjectSystem *system) {
    REGISTER_META_TYPE(AngelBehaviour);
    system->factoryAdd<AngelBehaviour>(gGeneral, AngelBehaviour::metaClass());
}

void AngelBehaviour::unregisterClassFactory(ObjectSystem *system) {
    UNREGISTER_META_TYPE(AngelBehaviour);
    system->factoryRemove<AngelBehaviour>(gGeneral);
}

VariantList AngelBehaviour::saveData() const {
    PROFILE_FUNCTION();
    return serializeData(metaObject());
}

void AngelBehaviour::loadData(const VariantList &data) {
    PROFILE_FUNCTION();
    Object::loadData(data);
}

void AngelBehaviour::setType(const TString &type) {
    PROFILE_FUNCTION();
    setScript(type);
}

void AngelBehaviour::setSystem(ObjectSystem *system) {
    Object::setSystem(system);
}

void AngelBehaviour::scriptSlot() {
    // Placeholder method  for the all incoming signals
}

void AngelBehaviour::onReferenceDestroyed() {
    Object *object = sender();

    for(auto &it : m_propertyFields) {
        if(it.second.object == object) {
            void *null = nullptr;
            memcpy(it.second.address, &null, sizeof(null));
        }
    }
}

Variant AngelBehaviour::readProperty(const MetaProperty &property) const {
    PROFILE_FUNCTION();

    auto it = m_propertyFields.find(property.name());
    if(it != m_propertyFields.end()) {
        const PropertyFields &fields = it->second;
        if(fields.isArray) {
            CScriptArray *array = reinterpret_cast<CScriptArray *>(fields.address);

            int typeId = array->GetElementTypeId();
            asITypeInfo *type = nullptr;
            if(typeId > asTYPEID_DOUBLE) {
                type = m_object->GetEngine()->GetTypeInfoById(typeId);

                if(type) {
                    if(TString(type->GetName()) == "string") {
                        typeId = MetaType::STRING;
                    } else {
                        typeId = MetaType::type(type->GetName());
                        if(type->GetFlags() & asOBJ_REF) {
                            typeId++;
                        }
                    }
                }
            } else {
                switch(typeId) {
                case asTYPEID_VOID:   typeId = MetaType::INVALID; break;
                case asTYPEID_BOOL:   typeId = MetaType::BOOLEAN; break;
                case asTYPEID_INT8:
                case asTYPEID_INT16:
                case asTYPEID_INT32:
                case asTYPEID_INT64:
                case asTYPEID_UINT8:
                case asTYPEID_UINT16:
                case asTYPEID_UINT32:
                case asTYPEID_UINT64: typeId = MetaType::INTEGER; break;
                case asTYPEID_FLOAT:
                case asTYPEID_DOUBLE: typeId = MetaType::FLOAT; break;
                default: break;
                }
            }

            VariantList list;
            for(int i = 0; i < array->GetSize(); i++) {
                switch(typeId) {
                    case MetaType::BOOLEAN: list.push_back(Variant(*reinterpret_cast<bool *>(array->At(i)))); break;
                    case MetaType::INTEGER: list.push_back(Variant(*reinterpret_cast<int *>(array->At(i)))); break;
                    case MetaType::FLOAT: list.push_back(Variant(*reinterpret_cast<float *>(array->At(i)))); break;
                    case MetaType::STRING: {
                        std::string *str = reinterpret_cast<std::string *>(array->At(i));
                        if(str) {
                            list.push_back(TString(*str));
                        }
                    } break;
                    default: {
                        if(type) {
                            if(fields.isScript) {
                                AngelBehaviour *behaviour = nullptr;
                                asIScriptObject *object = *(reinterpret_cast<asIScriptObject **>(array->At(i)));
                                if(object) {
                                    behaviour = reinterpret_cast<AngelBehaviour *>(object->GetUserData());
                                }
                                list.push_back(Variant(typeId, &behaviour));
                            } else {
                                void *ptr = *reinterpret_cast<void **>(array->At(i));
                                list.push_back(Variant(typeId, &ptr));
                            }
                        }
                    } break;
                }
            }
            return list;
        }
        if(fields.isScript) {
            if(fields.address) {
                AngelBehaviour *behaviour = nullptr;
                asIScriptObject *object = *(reinterpret_cast<asIScriptObject **>(fields.address));
                if(object) {
                    behaviour = reinterpret_cast<AngelBehaviour *>(object->GetUserData());
                }
                return Variant(MetaType::type(property.type().name()), &behaviour);
            }
        } else {
            return Variant(MetaType::type(property.type().name()), fields.address);
        }
    }
    return Variant();
}

void AngelBehaviour::writeProperty(const MetaProperty &property, const Variant &value) {
    PROFILE_FUNCTION();

    auto it = m_propertyFields.find(property.name());
    if(it != m_propertyFields.end()) {
        PropertyFields &fields = it->second;
        if(fields.isScript) {
            if(fields.isArray) {
                CScriptArray *array = reinterpret_cast<CScriptArray *>(fields.address);
                array->Resize(0);

                for(auto &element : *(reinterpret_cast<VariantList *>(value.data()))) {
                    AngelBehaviour *behaviour = nullptr;
                    if(element.type() == MetaType::INTEGER) {
                        uint32_t uuid = static_cast<uint32_t>(element.toInt());
                        if(uuid) {
                            behaviour = reinterpret_cast<AngelBehaviour *>(Engine::findObject(uuid));
                        }
                    } else {
                        behaviour = (element.data() == nullptr) ? nullptr : *(reinterpret_cast<AngelBehaviour **>(element.data()));
                    }

                    asIScriptObject *script = nullptr;
                    if(behaviour) {
                        connect(behaviour, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
                        script = behaviour->scriptObject();
                    }

                    array->InsertLast(&script);
                }
                return;
            } else {
                AngelBehaviour *behaviour = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<AngelBehaviour **>(value.data()));
                fields.object = behaviour;
                if(behaviour) {
                    asIScriptObject *script = behaviour->scriptObject();
                    behaviour->subscribe(this, fields.address);
                    memcpy(fields.address, &script, sizeof(script));
                    return;
                }
            }
        }
        if(fields.isObject) {
            if(fields.isArray) {
                CScriptArray *array = reinterpret_cast<CScriptArray *>(fields.address);
                array->Resize(0);

                for(auto &element : *(reinterpret_cast<VariantList *>(value.data()))) {
                    Object *object = nullptr;
                    if(element.type() == MetaType::INTEGER) {
                        uint32_t uuid = static_cast<uint32_t>(element.toInt());
                        if(uuid) {
                            object = Engine::findObject(uuid);
                        }
                    } else {
                        object = (element.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(element.data()));
                    }
                    if(object) {
                        connect(object, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
                    }

                    array->InsertLast(&object);
                }
                return;
            } else {
                Object *object = nullptr;
                if(value.type() == MetaType::INTEGER) {
                    uint32_t uuid = static_cast<uint32_t>(value.toInt());
                    if(uuid) {
                        object = Engine::findObject(uuid);
                    }
                } else {
                    object = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<Object **>(value.data()));
                }
                if(fields.object != object) {
                    disconnect(fields.object, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
                    fields.object = object;
                    connect(fields.object, _SIGNAL(destroyed()), this, _SLOT(onReferenceDestroyed()));
                }
            }
        } else if(fields.isArray) {
            CScriptArray *array = reinterpret_cast<CScriptArray *>(fields.address);
            array->Resize(0);

            for(auto &element : *(reinterpret_cast<VariantList *>(value.data()))) {
                if(element.userType() == MetaType::STRING) {
                    std::string str = element.toString().toStdString();
                    array->InsertLast(&str);
                } else {
                    array->InsertLast(element.data());
                }
            }
            return;
        }

        memcpy(fields.address, value.data(), MetaType(property.type()).size());
    }
}

void AngelBehaviour::methodCallEvent(MethodCallEvent *event) {
    PROFILE_FUNCTION();

    if(event) {
        const MetaObject *meta = metaObject();

        MetaMethod method = meta->method(event->method());
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
        memcpy(it.second, &m_object, sizeof(m_object));
    }
    m_obsevers.clear();
}
