#include "angelsystem.h"

#include <assert.h>

#include <log.h>

#include <angelscript.h>

#include <scriptarray/scriptarray.h>
#include <scriptgrid/scriptgrid.h>
#include <scriptdictionary/scriptdictionary.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptmath/scriptmath.h>

#include <systems/resourcesystem.h>

#ifdef SHARED_DEFINE
    #include <debugger/debugger.h>
#endif

#include <components/scene.h>
#include <components/actor.h>
#include <components/world.h>
#include <components/angelbehaviour.h>

#include <cstring>
#include <algorithm>

#include "resources/angelscript.h"

#include "bindings/angelbindings.h"

#define TEMPALTE "AngelBinary"
#define URI "thor://Components/"

void replace(std::string &srcStr, const std::string &findStr,
                               const std::string &replaceStr) {
    std::size_t replaceStrLen = replaceStr.length();
    for (std::size_t pos = 0; pos != std::string::npos; pos += replaceStrLen) {
        if ((pos = srcStr.find(findStr, pos)) != std::string::npos) {
            srcStr.replace(pos, findStr.length(), replaceStr);
        } else {
            break;
        }
    }
}

class AngelStream : public asIBinaryStream {
public:
    explicit AngelStream(ByteArray &ptr) :
            m_array(ptr),
            m_offset(0) {

    }
    int Write(const void *, asUINT) {
        return 0;
    }
    int Read(void *ptr, asUINT size) {
        if(size > 0) {
            memcpy(ptr, &m_array[m_offset], size);
            m_offset += size;
        }
        return size;
    }
protected:
    ByteArray &m_array;

    uint32_t m_offset;
};

AngelSystem::AngelSystem(Engine *engine) :
        System(),
        m_scriptEngine(nullptr),
        m_scriptModule(nullptr),
        m_context(nullptr),
        m_script(nullptr),
        m_inited(false) {
    PROFILE_FUNCTION();

    AngelBehaviour::registerClassFactory(this);

    AngelScript::registerClassFactory(engine->resourceSystem());

    setName("AngelScript");
}

AngelSystem::~AngelSystem() {
    PROFILE_FUNCTION();

    deleteAllObjects();

    if(m_context) {
        m_context->Release();
    }

    if(m_scriptModule) {
        m_scriptModule->Discard();
    }

    if(m_scriptEngine) {
        m_scriptEngine->ShutDownAndRelease();
    }

    AngelBehaviour::unregisterClassFactory(this);
}

bool AngelSystem::init() {
    PROFILE_FUNCTION();
    if(!m_inited) {
        m_scriptEngine = asCreateScriptEngine();

        int32_t r = m_scriptEngine->SetMessageCallback(asFUNCTION(messageCallback), nullptr, asCALL_CDECL);
        if(r >= 0) {
            m_context = m_scriptEngine->CreateContext();

            //registerClasses(m_scriptEngine);

            //reload();
        }
        m_inited = (r >= 0);
    }
    return m_inited;
}

void AngelSystem::reset() {
    if(Engine::isGameMode() && m_scriptModule) {
        m_scriptModule->ResetGlobalVars(m_context);
    }
}

void AngelSystem::update(World *world) {
    PROFILE_FUNCTION();

    if(Engine::isGameMode()) {
        for(auto it : m_objectList) {
            AngelBehaviour *component = static_cast<AngelBehaviour *>(it);
            asIScriptObject *object = component->scriptObject();
            if(object) {
                if(component->isEnabled()) {
                    Actor *actor = component->actor();
                    if(actor) {
                        Scene *scene = actor->scene();
                        if(scene && scene->parent() == world) {
                            if(!component->isStarted()) {
                                execute(object, component->scriptStart());
                                component->setStarted(true);
                            }
                            execute(object, component->scriptUpdate());
                        }
                    }
                }
                object->Release();
            }
        }
    }
}

int AngelSystem::threadPolicy() const {
    return Pool;
}

void AngelSystem::reload() {
    PROFILE_FUNCTION();

    unload();
    m_scriptModule = m_scriptEngine->GetModule("AngelData", asGM_CREATE_IF_NOT_EXISTS);

    if(Engine::isResourceExist(TEMPALTE)) {
        if(m_script) {
            Engine::reloadResource(TEMPALTE);
        } else {
            m_script = Engine::loadResource<AngelScript>(TEMPALTE);
            m_script->incRef();
        }
    } else {
        return;
    }

    if(m_script) {
        AngelStream stream(m_script->m_array);
        m_scriptModule->LoadByteCode(&stream);

        for(uint32_t i = 0; i < m_scriptModule->GetObjectTypeCount(); i++) {
            asITypeInfo *info = m_scriptModule->GetObjectTypeByIndex(i);
            if(info && isBehaviour(info)) {
                {
                    MetaType::Table staticTable = {
                        expose_props_method<AngelBehaviour>::exec(),
                        expose_method<AngelBehaviour>::exec(),
                        expose_enum<AngelBehaviour>::exec(),
                        TypeFuncs<AngelBehaviour>::size,
                        TypeFuncs<AngelBehaviour>::static_new,
                        TypeFuncs<AngelBehaviour>::construct,
                        TypeFuncs<AngelBehaviour>::static_delete,
                        TypeFuncs<AngelBehaviour>::destruct,
                        TypeFuncs<AngelBehaviour>::clone,
                        TypeFuncs<AngelBehaviour>::compare,
                        TypeFuncs<AngelBehaviour>::index,
                        info->GetName(),
                        MetaType::BASE_OBJECT
                    };

                    MetaType::registerType(staticTable);
                }

                {
                    int length = strlen(info->GetName());
                    char *type = new char[length + 3];
                    memcpy(type, info->GetName(), length);
                    type[length] = ' ';
                    type[length + 1] = '*';
                    type[length + 2] = 0;

                    MetaType::Table staticTable = {
                        expose_props_method<AngelBehaviour *>::exec(),
                        expose_method<AngelBehaviour *>::exec(),
                        expose_enum<AngelBehaviour *>::exec(),
                        TypeFuncs<AngelBehaviour *>::size,
                        TypeFuncs<AngelBehaviour *>::static_new,
                        TypeFuncs<AngelBehaviour *>::construct,
                        TypeFuncs<AngelBehaviour *>::static_delete,
                        TypeFuncs<AngelBehaviour *>::destruct,
                        TypeFuncs<AngelBehaviour *>::clone,
                        TypeFuncs<AngelBehaviour *>::compare,
                        TypeFuncs<AngelBehaviour *>::index,
                        type,
                        MetaType::POINTER | MetaType::BASE_OBJECT
                    };

                    MetaType::registerType(staticTable);
                }

                factoryAdd(info->GetName(), std::string(URI) + info->GetName(), AngelBehaviour::metaClass());
            }
        }
        for(auto it : m_objectList) {
            AngelBehaviour *behaviour = static_cast<AngelBehaviour *>(it);
            VariantMap data = behaviour->saveUserData();
            behaviour->createObject();
            behaviour->loadUserData(data);
        }
    } else {
        Log(Log::ERR) << __FUNCTION__ << "Filed to load a script";
    }
}

void *AngelSystem::execute(asIScriptObject *object, asIScriptFunction *func) {
    PROFILE_FUNCTION();

    if(func) {
        m_context->Prepare(func);
        if(object) {
            m_context->SetObject(object);
        }
        if(m_context->Execute() == asEXECUTION_EXCEPTION) {
            int column;
            m_context->GetExceptionLineNumber(&column);
            Log(Log::ERR) << __FUNCTION__ << "Unhandled Exception:" << m_context->GetExceptionString() << m_context->GetExceptionFunction()->GetName() << "Line:" << column;
        }
    } else {
        return nullptr;
    }
    return m_context->GetAddressOfReturnValue();
}

asIScriptModule *AngelSystem::module() const {
    PROFILE_FUNCTION();

    return m_scriptModule;
}

asIScriptContext *AngelSystem::context() const {
    PROFILE_FUNCTION();

    return m_context;
}

bool AngelSystem::isBehaviour(asITypeInfo *info) const {
    asITypeInfo *super = info->GetBaseType();
    if(super) {
        if(strcmp(super->GetName(), "Behaviour") == 0) {
            return true;
        }
        return isBehaviour(super);
    }
    return false;
}

void AngelSystem::unload() {
    if(m_scriptModule) {
        for(uint32_t i = 0; i < m_scriptModule->GetObjectTypeCount(); i++) {
            asITypeInfo *info = m_scriptModule->GetObjectTypeByIndex(i);
            if(info && isBehaviour(info)) {
                factoryRemove(info->GetName(), std::string(URI) + info->GetName());
            }
        }
        m_scriptModule->Discard();
    }
}

void *castTo(void *ptr) {
    return ptr;
}

void AngelSystem::registerClasses(asIScriptEngine *engine) {
    PROFILE_FUNCTION();

    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    RegisterScriptGrid(engine);
    RegisterScriptDictionary(engine);
    RegisterScriptMath(engine);

    registerTimer(engine);
    registerMath(engine);
    registerInput(engine);
    registerCore(engine);

    for(auto &it : MetaType::types()) {
        if(it.first > MetaType::USERTYPE) {
            MetaType::Table &table = it.second;
            const char *typeName = table.name;
            if(typeName[strlen(typeName) - 1] != '*') {
                engine->RegisterObjectType(table.name, 0, asOBJ_REF | asOBJ_NOCOUNT);
                std::string stream = std::string(table.name) + "@ f()";
                engine->RegisterObjectBehaviour(table.name, asBEHAVE_FACTORY, stream.c_str(), asFUNCTION(table.static_new), asCALL_CDECL);
                //engine->RegisterObjectBehaviour(name, asBEHAVE_ADDREF, "void f()", asMETHOD(CRef,AddRef), asCALL_THISCALL);
                //engine->RegisterObjectBehaviour(name, asBEHAVE_RELEASE, "void f()", asMETHOD(CRef,Release), asCALL_THISCALL);
            }
        }
    }

    for(auto &it : MetaType::types()) {
        if(it.first > MetaType::USERTYPE) {
            bindMetaType(engine, it.second);
        }
    }

    for(auto &it : System::factories()) {
        auto factory = System::metaFactory(it.first);
        if(factory) {
            bindMetaObject(engine, it.first, factory->first);
        }
    }

    engine->RegisterInterface("IBehaviour");
    engine->RegisterObjectMethod("AngelBehaviour",
                                 "void setScriptObject(IBehaviour @)",
                                 asMETHOD(AngelBehaviour, setScriptObject),
                                 asCALL_THISCALL);
    engine->RegisterObjectMethod("AngelBehaviour",
                                 "IBehaviour @scriptObject()",
                                 asMETHOD(AngelBehaviour, scriptObject),
                                 asCALL_THISCALL);

    engine->RegisterObjectMethod("Actor", "Actor &get_parent() property", asMETHOD(Actor, parent), asCALL_THISCALL);
    engine->RegisterObjectMethod("Actor", "void set_parent(Actor &) property", asMETHOD(Actor, setParent), asCALL_THISCALL);

    engine->RegisterObjectMethod("Actor", "string get_name() property", asMETHOD(Object, name), asCALL_THISCALL);
    engine->RegisterObjectMethod("Actor", "void set_name(string &in) property", asMETHOD(Object, setName), asCALL_THISCALL);

    registerEngine(engine);
}

void AngelSystem::bindMetaType(asIScriptEngine *engine, const MetaType::Table &table) {
    const char *typeName = table.name;
    if(typeName[strlen(typeName) - 1] != '*') {
        const MetaObject *meta;

        const MetaObject metaStruct (MetaObject(typeName, nullptr, nullptr,
                                     reinterpret_cast<const MetaMethod::Table *>(table.methods),
                                     reinterpret_cast<const MetaProperty::Table *>(table.properties),
                                     reinterpret_cast<const MetaEnum::Table *>(table.enums)));

        auto factory = System::metaFactory(typeName);
        if(factory) {
            meta = factory->first;
        } else {
            meta = &metaStruct;
        }

        for(int32_t e = 0; e < meta->enumeratorCount(); e++) {
            int offset = e - meta->enumeratorOffset();
            if(offset >= 0) {
                MetaEnum enumerator = meta->enumerator(e);
                if(enumerator.isValid()) {
                    const char *name = enumerator.name();
                    engine->RegisterEnum(name);
                    for(int32_t index = 0; index < enumerator.keyCount(); index++) {
                        engine->RegisterEnumValue(name, enumerator.key(index), enumerator.value(index));
                    }
                }
            }
        }

        for(int32_t m = 0; m < meta->methodCount(); m++) {
            MetaMethod method = meta->method(m);
            if(method.isValid()) {
                MetaType ret = method.returnType();
                std::string retName;
                if(ret.isValid()) {
                    retName = ret.name();
                    retName += ' ';
                } else {
                    retName = "void ";
                }

                if(method.table()->type == MetaMethod::Signal) {
                    continue;
                }

                std::string signature = retName + method.signature();
                for(auto &it : signature) {
                    if(it == '*') {
                        it = '@';
                    }
                }
                replace(signature, "std::", "");

                if(method.table()->type == MetaMethod::Static) {
                    engine->SetDefaultNamespace(typeName);

                    asSFuncPtr ptr(2);
                    method.table()->address(ptr.ptr.dummy, sizeof(void *));

                    engine->RegisterGlobalFunction(signature.c_str(), ptr, asCALL_CDECL);
                    engine->SetDefaultNamespace("");
                } else {
                    asSFuncPtr ptr(3);
                    method.table()->address(ptr.ptr.dummy, sizeof(void *));

                    engine->RegisterObjectMethod(typeName,
                                                 signature.c_str(),
                                                 ptr,
                                                 asCALL_THISCALL);
                }
            }
        }

        for(int32_t p = 0; p < meta->propertyCount(); p++) {
            MetaProperty property = meta->property(p);
            if(property.isValid()) {
                MetaType type = property.type();
                std::string name = type.name();

                bool ptr = false;
                bool skip = false;
                for(auto &it : name) {
                    if(it == '*') {
                        it = '&';
                    }
                    if(it == '&') {
                        ptr = true;
                    }
                    if(it == '<') { // Skip template types for now
                        skip = true;
                        break;
                    }
                }

                if(skip) {
                    continue;
                }

                //std::string ref = (ptr) ? " &" : "";
                std::string propertyName = property.name();
                replace(propertyName.begin(), propertyName.end(), '/', '_');
                int metaType = MetaType::type(type.name());
                std::string get = name + " get_" + propertyName + "() property";
                std::string set = std::string("void set_") + propertyName + "(" + name + ((metaType < MetaType::STRING) ? "" : (ptr ? "in" : "")) + ") property";

                asSFuncPtr ptr1(3); // 3 Means Method
                property.table()->readmem(ptr1.ptr.dummy, sizeof(void *));

                asSFuncPtr ptr2(3); // 3 Means Method
                property.table()->writemem(ptr2.ptr.dummy, sizeof(void *));

                engine->RegisterObjectMethod(typeName,
                                             get.c_str(),
                                             ptr1,
                                             asCALL_THISCALL);

                engine->RegisterObjectMethod(typeName,
                                             set.c_str(),
                                             ptr2,
                                             asCALL_THISCALL);
            }
        }
    }
}

void AngelSystem::bindMetaObject(asIScriptEngine *engine, const std::string &name, const MetaObject *meta) {
    const char *typeName = name.c_str();

    const MetaObject *super = meta->super();
    while(super != nullptr) {
        const char *superName = super->name();

        engine->RegisterObjectMethod(superName, (name + "@ opCast()").c_str(), asFUNCTION(castTo), asCALL_CDECL_OBJLAST);
        engine->RegisterObjectMethod(typeName, (std::string(superName) + "@ opImplCast()").c_str(), asFUNCTION(castTo), asCALL_CDECL_OBJLAST);

        super = super->super();
    }
}

void AngelSystem::messageCallback(const asSMessageInfo *msg, void *param) {
    PROFILE_FUNCTION();

    A_UNUSED(param);
    Log((Log::LogTypes)msg->type) << msg->section << "(" << msg->row << msg->col << "):" << msg->message;
}
