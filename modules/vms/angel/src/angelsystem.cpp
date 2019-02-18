#include "angelsystem.h"

#include <assert.h>

#include <log.h>

#include <analytics/profiler.h>

#include <angelscript.h>
#include <scriptstdstring/scriptstdstring.h>

#include <components/scene.h>
#include <components/actor.h>
#include <components/angelbehaviour.h>

#include <cstring>

#include "resources/angelscript.h"

#include "bindings/angelmath.h"
#include "bindings/angelcore.h"

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
        m_pContext(nullptr) {
    PROFILER_MARKER;

    AngelScript::registerClassFactory(this);

    AngelBehaviour::registerClassFactory(this);
}

AngelSystem::~AngelSystem() {
    PROFILER_MARKER;

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

    m_pScriptEngine = asCreateScriptEngine();

    int32_t r = m_pScriptEngine->SetMessageCallback(asFUNCTION(messageCallback), 0, asCALL_CDECL);
    if(r >= 0) {
        m_pContext = m_pScriptEngine->CreateContext();

        registerClasses(m_pScriptEngine);

        reload();
    }

    return (r >= 0);
}

const char *AngelSystem::name() const {
    return "AngelScript";
}

void AngelSystem::update(Scene *scene) {
    PROFILER_MARKER;

    for(auto it : m_List) {
        AngelBehaviour *component = static_cast<Component *>(it);
        if(component->isEnable() && component->actor()->scene() == scene) {
            asIScriptObject *object = component->scriptObject();
            string value    = component->script();
            if(object == nullptr && !value.empty() && m_pScriptModule) {
                asITypeInfo *type   = m_pScriptModule->GetTypeInfoByDecl(value.c_str());
                if(type) {
                    string stream   = value + " @+" + value + "()";
                    execute(object, type->GetFactoryByDecl(stream.c_str()));

                    asIScriptObject **obj   = (asIScriptObject**)m_pContext->GetAddressOfReturnValue();
                    if(obj == nullptr) {
                        continue;
                    }
                    object  = *obj;
                    if(object) {
                        object->AddRef();

                        component->setScriptObject(object);
                        component->setScriptStart(type->GetMethodByDecl("void start()"));
                        component->setScriptUpdate(type->GetMethodByDecl("void update()"));
                    } else {
                        Log(Log::ERR) << __FUNCTION__ << "Can't create an object" << value.c_str();
                    }
                }
            }

            if(Engine::isGameMode()) {
                if(!component->isStarted()) {
                    execute(object, component->scriptStart());
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
            Log(Log::ERR) << __FUNCTION__ << "Unhandled Exception:" << m_pContext->GetExceptionString();
        }
    }
}

void *castTo(void *ptr) {
    return ptr;
}

void AngelSystem::registerClasses(asIScriptEngine *engine) {
    PROFILER_MARKER;

    RegisterStdString(engine);

    registerMath(engine);
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
        const char *typeName    = it.first.c_str();
        const MetaObject *meta  = ISystem::metaFactory(typeName)->first;

        const MetaObject *super = meta->super();
        while(super != nullptr) {
            const char *superName   = super->name();

            engine->RegisterObjectMethod(superName, (it.first + "@ opCast()").c_str(), asFUNCTION(castTo), asCALL_CDECL_OBJLAST);
            engine->RegisterObjectMethod(typeName, (string(superName) + "@ opImplCast()").c_str(), asFUNCTION(castTo), asCALL_CDECL_OBJLAST);

            super = super->super();
        }

        uint32_t type   = MetaType::type(typeName);
        MetaType::Table *table  = MetaType::table(type);
        if(table) {
            for(uint32_t m = 0; m < meta->methodCount(); m++) {
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

            for(uint32_t p = 0; p < meta->propertyCount(); p++) {
                MetaProperty property = meta->property(p);
                if(property.isValid()) {

                    MetaType type   = property.type();
                    string name =  type.name();

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
}

void AngelSystem::messageCallback(const asSMessageInfo *msg, void *param) {
    PROFILER_MARKER;

    A_UNUSED(param)
    Log((Log::LogTypes)msg->type) << msg->section << "(" << msg->row << msg->col << "):" << msg->message;
}
