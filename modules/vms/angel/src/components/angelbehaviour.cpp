#include "components/angelbehaviour.h"

#include <resources/prefab.h>
#include <metamethod.h>

#include <cstring>

#include <log.h>

#include <angelscript.h>

#include "angelsystem.h"

namespace {
    const char *gResource("Resource");
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
    asIScriptModule *module = ptr->module();
    if(module) {
        asITypeInfo *type = module->GetTypeInfoByDecl(m_script.c_str());
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
    } else {
        Log(Log::ERR) << __FUNCTION__ << "The Script Module is NULL" << m_script;
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

            m_propertyFields.clear();

            asIScriptEngine *engine = m_object->GetEngine();
            uint32_t count = info->GetPropertyCount();
            for(uint32_t i = 0; i <= count; i++) {
                const char *name;
                int typeId;
                bool isPrivate;
                bool isProtected;
                info->GetProperty(i, &name, &typeId, &isPrivate, &isProtected);
                if(!isPrivate && !isProtected) {
                    PropertyFields propertyFields;
                    propertyFields.isScript = false;
                    propertyFields.isObject = false;
                    propertyFields.object = nullptr;
                    propertyFields.address = object->GetAddressOfProperty(i);
                    if(typeId > asTYPEID_DOUBLE) {
                        asITypeInfo *type = engine->GetTypeInfoById(typeId);
                        if(type) {
                            auto factory = System::metaFactory(type->GetName());
                            if(factory) {
                                propertyFields.isObject = true;
                            }

                            if(type->GetFlags() & asOBJ_SCRIPT_OBJECT) {
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
    return serializeData(AngelBehaviour::metaClass());
}

void AngelBehaviour::loadData(const VariantList &data) {
    PROFILE_FUNCTION();
    Object::loadData(data);
}

inline void trimmType(std::string &type, bool &isArray) {
    if(type.back() == '*') {
        type.pop_back();
        while(type.back() == ' ') {
            type.pop_back();
        }
    } else if(type.back() == ']') {
        type.pop_back();
        while(type.back() == ' ') {
            type.pop_back();
        }
        if(type.back() == '[') {
            type.pop_back();
            isArray = true;
        }
    }
}

void AngelBehaviour::setType(const std::string &type) {
    PROFILE_FUNCTION();
    setScript(type);
}

void AngelBehaviour::setSystem(ObjectSystem *system) {
    Object::setSystem(system);
}

void AngelBehaviour::scriptSlot() {
    // Method placeholder for the all incoming signals
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
        if(fields.isScript) {
            if(fields.address) {
                AngelBehaviour *behaviour = nullptr;
                asIScriptObject *object = *(reinterpret_cast<asIScriptObject **>(fields.address));
                if(object) {
                    behaviour = reinterpret_cast<AngelBehaviour *>(object->GetUserData());
                }
                return Variant(MetaType::type(property.table()->type->name), &behaviour);
            }
        } else {
            return Variant(MetaType::type(property.table()->type->name), fields.address);
        }
    }
    return Variant();
}

void AngelBehaviour::writeProperty(const MetaProperty &property, const Variant value) {
    PROFILE_FUNCTION();

    auto it = m_propertyFields.find(property.name());
    if(it != m_propertyFields.end()) {
        PropertyFields &fields = it->second;
        if(fields.isScript) {
            AngelBehaviour *behaviour = (value.data() == nullptr) ? nullptr : *(reinterpret_cast<AngelBehaviour **>(value.data()));
            fields.object = behaviour;
            if(behaviour) {
                asIScriptObject *script = *(reinterpret_cast<asIScriptObject **>(fields.address));
                if(script) {
                    script->Release();
                }
                script = behaviour->scriptObject();
                behaviour->subscribe(this, fields.address);
                memcpy(fields.address, &script, sizeof(script));
                return;
            }
        }
        if(fields.isObject) {
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

        memcpy(fields.address, value.data(), MetaType(property.table()->type).size());
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
        if(m_object) {
            m_object->AddRef();
        }
        memcpy(it.second, &m_object, sizeof(m_object));
    }
    m_obsevers.clear();
}
