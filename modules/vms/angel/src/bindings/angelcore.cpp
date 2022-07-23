#include "bindings/angelbindings.h"

#include <angelscript.h>

#include "engine.h"
#include "log.h"

#include "components/actor.h"

void logFunction(const string &in) {
    Log(Log::DBG) << in.c_str();
}

bool connect(Object *sender, const string &signal, Object *receiver, const string &slot) {
    return Object::connect(sender, signal.c_str(), receiver, slot.c_str());
}

void registerObject(asIScriptEngine *engine) {
    engine->RegisterObjectType("Object", 0, asOBJ_REF | asOBJ_NOCOUNT);

    engine->RegisterGlobalFunction("bool _connect(Object @, const string &in, Object @, const string &in)", asFUNCTION(connect), asCALL_CDECL);
}

Object *objectCreate1(const string &type) {
    return Engine::objectCreate(type);
}

Object *objectCreate2(const string &type, const string &name) {
    return Engine::objectCreate(type, name);
}

Object *objectCreate3(const string &type, const string &name, Object *parent) {
    return Engine::objectCreate(type, name, parent);
}

Actor *composeActor(const string &name, const string &component, Object *parent) {
    return Engine::composeActor(name, component, parent);
}

Object *loadResource(const string &name) {
    return Engine::loadResource(name);
}

void unloadResource(const string &name) {
    Engine::unloadResource(name);
}

void registerEngine(asIScriptEngine *engine) {
    engine->SetDefaultNamespace("Engine");

    engine->RegisterGlobalFunction("Object @objectCreate(const string &in)", asFUNCTION(objectCreate1), asCALL_CDECL);
    engine->RegisterGlobalFunction("Object @objectCreate(const string &in, const string &in)", asFUNCTION(objectCreate2), asCALL_CDECL);
    engine->RegisterGlobalFunction("Object @objectCreate(const string &in, const string &in, Object &in)", asFUNCTION(objectCreate3), asCALL_CDECL);

    engine->RegisterGlobalFunction("Actor @composeActor(const string &in, const string &in, Object &in)", asFUNCTION(composeActor), asCALL_CDECL);

    engine->RegisterGlobalFunction("Object @loadResource(const string &in)", asFUNCTION(loadResource), asCALL_CDECL);
    engine->RegisterGlobalFunction("void unloadResource(const string &in)", asFUNCTION(unloadResource), asCALL_CDECL);

    engine->RegisterGlobalFunction("Scene @loadScene(const string &in, bool)", asFUNCTION(Engine::loadScene), asCALL_CDECL);
    engine->RegisterGlobalFunction("void unloadScene(Scene &)", asFUNCTION(Engine::unloadScene), asCALL_CDECL);

    engine->SetDefaultNamespace("");
}

void registerCore(asIScriptEngine *engine) {
    engine->RegisterGlobalFunction("void log(const string &in)", asFUNCTION(logFunction), asCALL_CDECL);

    registerObject(engine);
}
