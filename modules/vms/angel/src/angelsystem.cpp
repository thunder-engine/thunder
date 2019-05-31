#include "angelsystem.h"

#include <assert.h>

#include <log.h>

#include <analytics/profiler.h>

#include <angelscript.h>
#include <scriptarray/scriptarray.h>
#include <scriptstdstring/scriptstdstring.h>

#include <components/scene.h>
#include <components/actor.h>
#include <components/angelbehaviour.h>

#include <cstring>

#include "resources/angelscript.h"

#include "bindings/angelbindings.h"

#define TEMPALTE "{00000000-0101-0000-0000-000000000000}"

class AngelStream : public asIBinaryStream {
public:
    AngelStream(ByteArray &ptr) :
            m_Array(ptr),
            m_Offset(0) {

    }
    int         Write   (const void *, asUINT) {
        return 0;
    }
    int         Read    (void *ptr, asUINT size) {
        if(size > 0) {
            memcpy(ptr, &m_Array[m_Offset], size);
            m_Offset    += size;
        }
        return size;
    }
protected:
    ByteArray          &m_Array;

    uint32_t            m_Offset;
};

AngelSystem::AngelSystem() :
        ISystem(),
        m_pScriptEngine(nullptr),
        m_pScriptModule(nullptr),
        m_pContext(nullptr),
        m_Inited(false) {
    PROFILER_MARKER;

    AngelScript::registerClassFactory(this);

    AngelBehaviour::registerClassFactory(this);
}

AngelSystem::~AngelSystem() {
    PROFILER_MARKER;

    auto it = m_List.begin();
    while(it != m_List.end()) {
        delete *it;
        it = m_List.begin();
    }
    m_List.clear();

    if(m_pContext) {
        m_pContext->Release();
    }

    if(m_pScriptModule) {
        m_pScriptModule->Discard();
    }

    if(m_pScriptEngine) {
        m_pScriptEngine->ShutDownAndRelease();
    }

    AngelBehaviour::unregisterClassFactory(this);

    AngelScript::unregisterClassFactory(this);
}

bool AngelSystem::init() {
    PROFILER_MARKER;
    if(!m_Inited) {
        m_pScriptEngine = asCreateScriptEngine();

        int32_t r = m_pScriptEngine->SetMessageCallback(asFUNCTION(messageCallback), nullptr, asCALL_CDECL);
        if(r >= 0) {
            m_pContext = m_pScriptEngine->CreateContext();

            registerClasses(m_pScriptEngine);

            reload();
        }
        m_Inited = (r >= 0);
    }
    return m_Inited;
}

const char *AngelSystem::name() const {
    return "AngelScript";
}

void AngelSystem::update(Scene *scene) {
    PROFILER_MARKER;

    if(Engine::isGameMode()) {
        for(auto it : m_List) {
            AngelBehaviour *component = static_cast<AngelBehaviour *>(it);
            asIScriptObject *object = component->scriptObject();
            if(component->isEnable() && component->actor() && component->actor()->scene() == scene) {
                if(!component->isStarted()) {
                    execute(object, component->scriptStart());
                    component->setStarted(true);
                }
                execute(object, component->scriptUpdate());
            }
        }
    }
}

void AngelSystem::reload() {
    PROFILER_MARKER;

    if(m_pScriptModule) {
        m_pScriptModule->Discard();

        Engine::unloadResource(TEMPALTE);
    }

    AngelScript *script = Engine::loadResource<AngelScript>(TEMPALTE);
    if(script) {
        AngelStream stream(script->m_Array);
        m_pScriptModule = m_pScriptEngine->GetModule("AngelData", asGM_CREATE_IF_NOT_EXISTS);
        m_pScriptModule->LoadByteCode(&stream);

        //const MetaObject *meta = AngelBehaviour::metaClass();

        for(uint32_t i = 0; i < m_pScriptModule->GetObjectTypeCount(); i++) {
            asITypeInfo *info = m_pScriptModule->GetObjectTypeByIndex(i);
            if(info) {
                asITypeInfo *super = info->GetBaseType();
                if(super && strcmp(super->GetName(), "Behaviour") == 0) {
                    //registerMetaType(m_pScriptEngine, info->GetName(), meta);
                }
            }
        }
    } else {
        Log(Log::ERR) << __FUNCTION__ << "Filed to load a script";
    }
}

void AngelSystem::execute(asIScriptObject *object, asIScriptFunction *func) {
    PROFILER_MARKER;

    if(func) {
        m_pContext->Prepare(func);
        if(object) {
            m_pContext->SetObject(object);
        }
        if(m_pContext->Execute() == asEXECUTION_EXCEPTION) {
            int column;
            m_pContext->GetExceptionLineNumber(&column);
            Log(Log::ERR) << __FUNCTION__ << "Unhandled Exception:" << m_pContext->GetExceptionString() << m_pContext->GetExceptionFunction()->GetName() << "Line:" << column;
        }
    }
}

asIScriptModule *AngelSystem::module() const {
    PROFILER_MARKER;

    return m_pScriptModule;
}

asIScriptContext *AngelSystem::context() const {
    PROFILER_MARKER;

    return m_pContext;
}

void *castTo(void *ptr) {
    return ptr;
}

void AngelSystem::registerClasses(asIScriptEngine *engine) {
    PROFILER_MARKER;

    RegisterStdString(engine);
    RegisterScriptArray(engine, true);

    registerTimer(engine);
    registerMath(engine);
    registerInput(engine);
    registerCore(engine);

    for(auto &it : MetaType::types()) {
        if(it.first > MetaType::USERTYPE) {
            MetaType::Table &table  = it.second;
            MetaType type(&table);
            const char *name        = type.name();
            if(name[strlen(name) - 1] != '*') {
                engine->RegisterObjectType(name, 0, asOBJ_REF | asOBJ_NOCOUNT);
                string stream   = string(name) + "@ f()";
                engine->RegisterObjectBehaviour(name, asBEHAVE_FACTORY, stream.c_str(), asFUNCTION(table.static_new), asCALL_CDECL);
                //engine->RegisterObjectBehaviour(name, asBEHAVE_ADDREF, "void f()", asMETHOD(CRef,AddRef), asCALL_THISCALL);
                //engine->RegisterObjectBehaviour(name, asBEHAVE_RELEASE, "void f()", asMETHOD(CRef,Release), asCALL_THISCALL);
            }
        }
    }

    for(auto &it: ISystem::factories()) {
        auto factory = ISystem::metaFactory(it.first);
        if(factory) {
            registerMetaType(engine, it.first, factory->first);
        }
    }

    engine->RegisterInterface("IBehaviour");
    engine->RegisterObjectMethod("AngelBehaviour",
                                 "void setScriptObject(IBehaviour @)",
                                 asMETHOD(AngelBehaviour, setScriptObject),
                                 asCALL_THISCALL);

    engine->RegisterObjectMethod("Actor", "Actor &get_Parent()", asMETHOD(Actor, parent), asCALL_THISCALL);
    engine->RegisterObjectMethod("Actor", "void set_Parent(Actor &)", asMETHOD(Actor, setParent), asCALL_THISCALL);

    engine->RegisterObjectMethod("Actor", "string &get_Name()", asMETHOD(Object, name), asCALL_THISCALL);
    engine->RegisterObjectMethod("Actor", "void set_Name(string &in)", asMETHOD(Object, setName), asCALL_THISCALL);
}

void AngelSystem::registerMetaType(asIScriptEngine *engine, const string &name, const MetaObject *meta) {
    const char *typeName    = name.c_str();

    const MetaObject *super = meta->super();
    while(super != nullptr) {
        const char *superName   = super->name();

        engine->RegisterObjectMethod(superName, (name + "@ opCast()").c_str(), asFUNCTION(castTo), asCALL_CDECL_OBJLAST);
        engine->RegisterObjectMethod(typeName, (string(superName) + "@ opImplCast()").c_str(), asFUNCTION(castTo), asCALL_CDECL_OBJLAST);

        super = super->super();
    }

    uint32_t type = MetaType::type(typeName);
    MetaType::Table *table  = MetaType::table(type);
    if(table) {
        for(int32_t m = 0; m < meta->methodCount(); m++) {
            MetaMethod method   = meta->method(m);
            if(method.isValid()) {
                MetaType ret    = method.returnType();
                string retName;
                if(ret.isValid()) {
                    retName = ret.name();
                    retName += ' ';
                } else {
                    retName = "void ";
                }

                asSFuncPtr ptr(3);
                method.table()->address(ptr.ptr.dummy, sizeof(void *));

                string signature    = retName + method.signature();
                for(auto &it : signature) {
                    if(it == '*') {
                        it = '@';
                    }
                }

                engine->RegisterObjectMethod(typeName,
                                             signature.c_str(),
                                             ptr,
                                             asCALL_THISCALL);
            }
        }

        for(int32_t p = 0; p < meta->propertyCount(); p++) {
            MetaProperty property = meta->property(p);
            if(property.isValid()) {

                MetaType type = property.type();
                string name = type.name();

                bool ptr    = false;
                for(auto &it : name) {
                    if(it == '*') {
                        it = '&';
                    }
                    if(it == '&') {
                        ptr = true;
                    }
                }

                string ref  = (ptr) ? "" : " &";

                string get  = name + ref +"get_" + property.name() + "()";
                string set  = string("void set_") + property.name() + "(" + name + ((MetaType::type(type.name()) < MetaType::STRING) ? "" : (ref + ((ptr) ? "" : "in"))) + ")";

                asSFuncPtr ptr1(3);
                property.table()->readmem(ptr1.ptr.dummy, sizeof(void *));

                asSFuncPtr ptr2(3);
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

void AngelSystem::messageCallback(const asSMessageInfo *msg, void *param) {
    PROFILER_MARKER;

    A_UNUSED(param)
    Log((Log::LogTypes)msg->type) << msg->section << "(" << msg->row << msg->col << "):" << msg->message;
}
