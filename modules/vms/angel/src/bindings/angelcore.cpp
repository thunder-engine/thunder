#include "bindings/angelbindings.h"

#include <angelscript.h>

#include "engine.h"
#include "input.h"
#include "log.h"

#include "components/actor.h"

void registerObject(asIScriptEngine *engine) {
    engine->RegisterObjectType("Object", 0, asOBJ_REF | asOBJ_NOCOUNT);
}

Object *objectCreate1(string &type) {
    return Engine::objectCreate(type);
}

Object *objectCreate2(string &type, string &name) {
    return Engine::objectCreate(type, name);
}

Object *objectCreate3(string &type, string &name, Object *parent) {
    return Engine::objectCreate(type, name, parent);
}

Actor *actorCreate(string &name, Object *parent) {
    return Engine::objectCreate<Actor>(name, parent);
}

void registerEngine(asIScriptEngine *engine) {
    engine->SetDefaultNamespace("Engine");

    engine->RegisterGlobalFunction("Object @objectCreate(const string &in)", asFUNCTION(objectCreate1), asCALL_CDECL);
    engine->RegisterGlobalFunction("Object @objectCreate(const string &in, const string &in)", asFUNCTION(objectCreate2), asCALL_CDECL);
    engine->RegisterGlobalFunction("Object @objectCreate(const string &in, const string &in, Object &in)", asFUNCTION(objectCreate3), asCALL_CDECL);

    //engine->RegisterGlobalFunction("Actor @actorCreate(const string &in, Object &in)", asFUNCTION(actorCreate), asCALL_CDECL);

    engine->RegisterGlobalFunction("Object @loadResource(const string &in)", asFUNCTION(Engine::loadResourceImpl), asCALL_CDECL);

    engine->SetDefaultNamespace("");
}

void logFunction(string &in) {
    Log(Log::DBG) << in.c_str();
}

void registerCore(asIScriptEngine *engine) {
    engine->RegisterGlobalFunction("void log(const string &in)", asFUNCTION(logFunction), asCALL_CDECL);

    registerObject(engine);

    registerEngine(engine);
}
