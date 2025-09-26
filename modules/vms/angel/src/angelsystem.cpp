#include "angelsystem.h"

#include <assert.h>

#include <log.h>

#include <angelscript.h>
#include <autowrapper/aswrappedcall.h>

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

#include <cstring>
#include <algorithm>

#include "resources/angelscript.h"

#include "bindings/angelbindings.h"

namespace {
    const char *gTemplate("AngelBinary");
    const char *gUri("thor://Components/");
    const char *gLabel("[AngelScript]");
}

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
        m_inited(false),
        m_generic(false) {
    PROFILE_FUNCTION();

    AngelBehaviour::registerClassFactory(this);

    AngelScript::registerClassFactory(engine->resourceSystem());

    setName("AngelScript");

    m_generic = strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY");
}

AngelSystem::~AngelSystem() {
    PROFILE_FUNCTION();

    deleteAllObjects();

    unload();

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
            registerClasses(m_scriptEngine);

            reload();
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
                    Scene *scene = component->scene();
                    if(scene && scene->parent() == world) {
                        if(!component->isStarted()) {
                            execute(object, component->scriptStart());
                            component->setStarted(true);
                        }
                        execute(object, component->scriptUpdate());
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

    if(Engine::isResourceExist(gTemplate)) {
        if(m_script) {
            Engine::reloadResource(gTemplate);
        } else {
            m_script = Engine::loadResource<AngelScript>(gTemplate);
            if(m_script) {
                m_script->incRef();
            }
        }
    } else {
        return;
    }

    m_context = m_scriptEngine->CreateContext();

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

                factoryAdd(info->GetName(), std::string(gUri) + info->GetName(), AngelBehaviour::metaClass());
            }
        }

        processEvents();

        for(auto it : m_objectList) {
            AngelBehaviour *behaviour = static_cast<AngelBehaviour *>(it);
            VariantMap data = behaviour->saveUserData();
            behaviour->createObject();
            behaviour->loadUserData(data);
        }
    } else {
        aError() << __FUNCTION__ << "Filed to load a script";
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
            aError() << __FUNCTION__ << "Unhandled Exception:" << m_context->GetExceptionString() << m_context->GetExceptionFunction()->GetName() << "Line:" << column;
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

MetaObject *AngelSystem::getMetaObject(asIScriptObject *object) {
    asITypeInfo *info = object->GetObjectType();

    auto it = m_metaObjects.find(info);
    if(it != m_metaObjects.end()) {
        return it->second;
    }

    std::vector<MetaProperty::Table> propertyTable;
    std::vector<MetaMethod::Table> methodTable;

    uint32_t count = info->GetPropertyCount();
    for(uint32_t i = 0; i <= count; i++) {
        if(i == count) {
            propertyTable.push_back({nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr});
        } else {
            const char *name;
            int typeId;
            bool isPrivate;
            bool isProtected;
            info->GetProperty(i, &name, &typeId, &isPrivate, &isProtected);
            if(!isPrivate && !isProtected) {
                TString typeName;
                uint32_t metaType = 0;
                if(typeId > asTYPEID_DOUBLE) {
                    asITypeInfo *type = m_scriptEngine->GetTypeInfoById(typeId);
                    if(type) {
                        typeName = type->GetName();
                        if(typeName == "array") {
                            typeId = type->GetSubTypeId();
                            if(typeId > asTYPEID_DOUBLE) {
                                asITypeInfo *subType = type->GetSubType();
                                typeName = TString(subType->GetName()) + "[]";
                            } else {
                                // Implement array with base type
                                switch(typeId) {
                                case asTYPEID_VOID:   metaType = MetaType::INVALID; break;
                                case asTYPEID_BOOL:   typeName = "bool[]"; break;
                                case asTYPEID_INT8:
                                case asTYPEID_INT16:
                                case asTYPEID_INT32:
                                case asTYPEID_INT64:
                                case asTYPEID_UINT8:
                                case asTYPEID_UINT16:
                                case asTYPEID_UINT32:
                                case asTYPEID_UINT64: typeName = "int[]"; break;
                                case asTYPEID_FLOAT:
                                case asTYPEID_DOUBLE: typeName = "float[]"; break;
                                default: break;
                                }
                            }
                        } else {
                            if(type->GetFlags() & asOBJ_REF) {
                                typeName += " *";
                            }
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
                const MetaType::Table *table = nullptr;
                if(!typeName.isEmpty()) {
                    typeName.replace("string", "TString");

                    table = AngelSystem::metaType(typeName);
                } else {
                    table = MetaType::table(metaType);
                }

                propertyTable.push_back({name, table, nullptr, nullptr, nullptr, nullptr, nullptr,
                                         &Reader<decltype(&AngelBehaviour::readProperty), &AngelBehaviour::readProperty>::read,
                                         &Writer<decltype(&AngelBehaviour::writeProperty), &AngelBehaviour::writeProperty>::write});
            }
        }
    }

    count = info->GetMethodCount();
    for(uint32_t m = 0; m <= count; m++) {
        if(m == count) {
            methodTable.push_back({MetaMethod::Method, nullptr, nullptr, nullptr, 0, 0, nullptr});
        } else {
            asIScriptFunction *method = info->GetMethodByIndex(m);
            if(method) {
                std::string name(method->GetName());
                if(name.size() > 2 && name[0] == 'o' && name[1] == 'n') { // this is a slot
                    MetaMethod::Table table = A_SLOTEX(AngelBehaviour::scriptSlot, method->GetName());
                    table.sign = Mathf::hashString(std::string(method->GetName()) + "()");
                    methodTable.push_back(table);
                }
            }
        }
    }

    MetaProperty::Table *props = new MetaProperty::Table[propertyTable.size()];
    memcpy(props, propertyTable.data(), sizeof(MetaProperty::Table) * propertyTable.size());

    MetaMethod::Table *metods = new MetaMethod::Table[methodTable.size()];
    memcpy(metods, methodTable.data(), sizeof(MetaMethod::Table) * methodTable.size());

    MetaObject *metaObject = new MetaObject(info->GetName(), AngelBehaviour::metaClass(),
                                            &AngelBehaviour::construct, metods, props, nullptr);

    m_metaObjects[info] = metaObject;

    return metaObject;
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
                factoryRemove(info->GetName(), TString(gUri) + info->GetName());
            }
        }
        m_scriptModule->Discard();
        m_scriptModule = nullptr;
    }

    if(m_context) {
        if(m_context->GetState() == asEXECUTION_ACTIVE) {
            m_context->Abort();
        }
        m_context->Release();
        m_context = nullptr;
    }

    for(auto it : m_metaObjects) {
        // need to delete props and methods as well
        delete it.second;
    }
    m_metaObjects.clear();

    for(auto it : m_metaTypes) {
        delete []it.second->dynamicName;
        delete it.second;
    }
    m_metaTypes.clear();
}

Object *castTo(Object *ptr) {
    return ptr;
}

Variant getArgument(asIScriptGeneric *gen, int index) {
    int type = gen->GetArgTypeId(index);

    if(type & asTYPEID_MASK_OBJECT) {
        asITypeInfo *info = gen->GetEngine()->GetTypeInfoById(type);
        if(info) {
            uint32_t metaType = MetaType::type(info->GetName());
            if(metaType != MetaType::INVALID) {
                return Variant(metaType, gen->GetArgObject(index));
            }
        }
    } else {
        switch(type) {
            case asTYPEID_BOOL: { return gen->GetArgByte(index); }
            case asTYPEID_UINT32:
            case asTYPEID_INT32: { return static_cast<int>(gen->GetArgDWord(index)); }
            case asTYPEID_DOUBLE:
            case asTYPEID_FLOAT: { return gen->GetArgFloat(index); }
            default: break;
        }
    }

    return Variant();
}

void setResult(asIScriptGeneric *gen, const Variant &value) {
    if(value.isValid()) {
        switch(value.type()) {
            case MetaType::BOOLEAN: {
                gen->SetReturnByte(value.toBool());
            } break;
            case MetaType::INTEGER: {
                gen->SetReturnDWord(value.toInt());
            } break;
            case MetaType::FLOAT: {
                gen->SetReturnFloat(value.toFloat());
            } break;
            case MetaType::VECTOR2:
            case MetaType::VECTOR3:
            case MetaType::VECTOR4:
            case MetaType::QUATERNION:
            case MetaType::MATRIX3:
            case MetaType::MATRIX4:
            case MetaType::RAY:
            case MetaType::STRING: {
                gen->SetReturnObject(value.data());
            } break;
            case MetaType::OBJECT:
            default: {
                gen->SetReturnObject(*reinterpret_cast<void **>(value.data()));
            } break;
        }
    }
}

void wrapGeneric(asIScriptGeneric *gen) {
    Object *obj = reinterpret_cast<Object *>(gen->GetObject());
    if(obj) {
        asIScriptFunction *function = gen->GetFunction();
        if(function) {
            const MetaObject *meta = obj->metaObject();

            bool hasParam = false;
            std::string params = "(";
            for(int i = 0; i < function->GetParamCount(); i++) {
                int typeId = 0;
                asDWORD flags = 0;
                function->GetParam(i, &typeId, &flags);

                asITypeInfo *info = gen->GetEngine()->GetTypeInfoById(typeId);
                if(info) {
                    std::string typeName(info->GetName());
                    if(typeName == "string") {
                        typeName.insert(0, "std::");
                    }
                    params += typeName + ",";
                    hasParam = true;
                }
            }
            if(hasParam) {
                params.pop_back();
            }
            params += ')';

            std::string sig(function->GetName() + params);
            int index = meta->indexOfMethod(sig.c_str());
            if(index > -1) {
                MetaMethod method = meta->method(index);

                std::vector<Variant> args;
                args.reserve(gen->GetArgCount());
                for(int i = 0; i < gen->GetArgCount(); i++) {
                    args.push_back(getArgument(gen, i));
                }

                Variant returnValue;
                if(!method.invoke(obj, returnValue, args.size(), args.data())) {
                    aDebug() << gLabel << "Unable to call method" << function->GetName() << "for" << obj;
                } else {
                    setResult(gen, returnValue);
                }
            }
        }
    }
}

void wrapGetGeneric(asIScriptGeneric *gen) {
    Object *obj = reinterpret_cast<Object *>(gen->GetObject());
    if(obj) {
        asIScriptFunction *function = gen->GetFunction();
        if(function) {
            const char *functionName = (&function->GetName()[4]);
            setResult(gen, obj->property(functionName));
        }
    }
}

void wrapSetGeneric(asIScriptGeneric *gen) {
    Object *obj = reinterpret_cast<Object *>(gen->GetObject());
    if(obj) {
        asIScriptFunction *function = gen->GetFunction();
        if(function) {
            const char *functionName = (&function->GetName()[4]);
            Variant value = getArgument(gen, 0);
            if(value.isValid()) {
                obj->setProperty(functionName, value);
            }
        }
    }
}

void AngelSystem::registerClasses(asIScriptEngine *engine) {
    PROFILE_FUNCTION();

    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    if(!m_generic) {
        RegisterScriptGrid(engine);
    }
    RegisterScriptDictionary(engine);
    RegisterScriptMath(engine);

    // Core
    registerLog(engine, m_generic);
    registerString(engine, m_generic);
    registerObject(engine, m_generic);
    // Math
    registerMath(engine, m_generic);
    // Engine
    registerTimer(engine, m_generic);
    registerInput(engine, m_generic);

    for(auto &it : MetaType::types()) {
        if(it.first > MetaType::USERTYPE) {
            MetaType::Table &table = it.second;
            const char *typeName = table.name;
            if(typeName[strlen(typeName) - 1] != '*') {
                engine->RegisterObjectType(table.name, 0, asOBJ_REF | asOBJ_NOCOUNT);
                std::string stream = std::string(table.name) + "@ f()";

                engine->RegisterObjectBehaviour(table.name,
                                                asBEHAVE_FACTORY,
                                                stream.c_str(),
                                                m_generic ? asFUNCTION(wrapGeneric) : asFUNCTION(table.static_new),
                                                m_generic ? asCALL_GENERIC : asCALL_CDECL);

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
                                 m_generic ? WRAP_MFN(AngelBehaviour, setScriptObject) : asMETHOD(AngelBehaviour, setScriptObject),
                                 m_generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("AngelBehaviour",
                                 "IBehaviour @scriptObject()",
                                 m_generic ? WRAP_MFN(AngelBehaviour, scriptObject) : asMETHOD(AngelBehaviour, scriptObject),
                                 m_generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Actor",
                                 "Actor &get_parent() property",
                                 m_generic ? WRAP_MFN(Actor, parent) : asMETHOD(Actor, parent),
                                 m_generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Actor",
                                 "void set_parent(Actor &) property",
                                 m_generic ? WRAP_MFN(Actor, setParent) : asMETHOD(Actor, setParent),
                                 m_generic ? asCALL_GENERIC : asCALL_THISCALL);

    registerEngine(engine, m_generic);
}

void AngelSystem::bindMetaType(asIScriptEngine *engine, const MetaType::Table &table) {
    TString typeName = table.name;

    if(typeName.back() != '*') {
        const MetaObject *meta;

        const MetaObject metaStruct (MetaObject(typeName.data(), nullptr, nullptr,
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
                    engine->SetDefaultNamespace(typeName.data());

                    asSFuncPtr ptr(2);
                    method.table()->address(ptr.ptr.dummy, sizeof(void *));

                    engine->RegisterGlobalFunction(signature.c_str(),
                                                   m_generic ? asFUNCTION(wrapGeneric) : ptr,
                                                   m_generic ? asCALL_GENERIC : asCALL_CDECL);
                    engine->SetDefaultNamespace("");
                } else {
                    asSFuncPtr ptr(3);
                    method.table()->address(ptr.ptr.dummy, sizeof(void *));

                    engine->RegisterObjectMethod(typeName.data(),
                                                 signature.c_str(),
                                                 m_generic ? asFUNCTION(wrapGeneric) : ptr,
                                                 m_generic ? asCALL_GENERIC : asCALL_THISCALL);
                }
            }
        }

        for(int32_t p = 0; p < meta->propertyCount(); p++) {
            MetaProperty property = meta->property(p);
            if(property.isValid()) {
                MetaType type = property.type();
                std::string propertyTypeName = type.name();

                bool isArray = false;
                bool isObject = false;

                for(auto &it : propertyTypeName) {
                    if(it == '*') {
                        it = '&';
                    }
                    if(it == '&') {
                        isObject = true;
                    }
                }

                if(propertyTypeName.back() == ']') {
                    propertyTypeName.pop_back();
                    while(propertyTypeName.back() == ' ') {
                        propertyTypeName.pop_back();
                    }
                    if(propertyTypeName.back() == '[') {
                        propertyTypeName.pop_back();
                        isArray = true;
                    }
                }

                if(isArray) {
                    continue;
                }

                std::string propertyName = property.name();
                replace(propertyName.begin(), propertyName.end(), '/', '_');
                int metaType = MetaType::type(type.name());
                std::string get = propertyTypeName + " get_" + propertyName + "() property";
                std::string set = std::string("void set_") + propertyName + "(" + propertyTypeName + ((metaType < MetaType::STRING) ? "" : (isObject ? "in" : "")) + ") property";

                asSFuncPtr ptr1(3); // 3 Means Method
                property.table()->readmem(ptr1.ptr.dummy, sizeof(void *));

                engine->RegisterObjectMethod(typeName.data(),
                                             get.c_str(),
                                             m_generic ? asFUNCTION(wrapGetGeneric) : ptr1,
                                             m_generic ? asCALL_GENERIC : asCALL_THISCALL);

                asSFuncPtr ptr2(3); // 3 Means Method
                property.table()->writemem(ptr2.ptr.dummy, sizeof(void *));

                engine->RegisterObjectMethod(typeName.data(),
                                             set.c_str(),
                                             m_generic ? asFUNCTION(wrapSetGeneric) : ptr2,
                                             m_generic ? asCALL_GENERIC : asCALL_THISCALL);
            }
        }
    }
}

void AngelSystem::bindMetaObject(asIScriptEngine *engine, const TString &name, const MetaObject *meta) {
    const char *typeName = name.data();

    const MetaObject *super = meta->super();
    while(super != nullptr) {
        const char *superName = super->name();

        engine->RegisterObjectMethod(superName,
                                     (name + "@ opCast()").data(),
                                     m_generic ? WRAP_OBJ_LAST(castTo) : asFUNCTION(castTo),
                                     m_generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

        engine->RegisterObjectMethod(typeName,
                                     (std::string(superName) + "@ opImplCast()").c_str(),
                                     m_generic ? WRAP_OBJ_LAST(castTo) : asFUNCTION(castTo),
                                     m_generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

        super = super->super();
    }
}

MetaType::Table *AngelSystem::metaType(const TString &typeName) {
    MetaType::Table *table = MetaType::table(MetaType::type(typeName.data()));
    if(table == nullptr) {
        auto it = m_metaTypes.find(typeName);
        if(it != m_metaTypes.end()) {
            return it->second;
        } else {
            table = new MetaType::Table();
            table->dynamicName = new char[typeName.size() + 1];
            memcpy(table->dynamicName, typeName.data(), typeName.size() + 1);

            m_metaTypes[typeName] = table;
        }
    }

    return table;
}

void AngelSystem::messageCallback(const asSMessageInfo *msg, void *param) {
    PROFILE_FUNCTION();

    A_UNUSED(param);
    Log((Log::LogTypes)msg->type) << gLabel << msg->section << "(" << msg->row << msg->col << "):" << msg->message;
}
