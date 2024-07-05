#include "bindings/angelbindings.h"

#include <angelscript.h>

#include "engine.h"
#include "log.h"

#include "components/actor.h"

void debugFunction(const std::string &in) {
    aDebug() << in.c_str();
}

void infoFunction(const std::string &in) {
    aInfo() << in.c_str();
}

void warningFunction(const std::string &in) {
    aWarning() << in.c_str();
}

void errorFunction(const std::string &in) {
    aError() << in.c_str();
}

void criticalFunction(const std::string &in) {
    aCritical() << in.c_str();
}

bool connect(Object *sender, const std::string &signal, Object *receiver, const std::string &slot) {
    return Object::connect(sender, signal.c_str(), receiver, slot.c_str());
}

void registerObject(asIScriptEngine *engine) {
    engine->RegisterObjectType("Object", 0, asOBJ_REF | asOBJ_NOCOUNT);

    engine->RegisterGlobalFunction("bool _connect(Object @, const std::string &in, Object @, const std::string &in)", asFUNCTION(connect), asCALL_CDECL);
}

Object *objectCreate1(const std::string &type) {
    return Engine::objectCreate(type);
}

Object *objectCreate2(const std::string &type, const std::string &name) {
    return Engine::objectCreate(type, name);
}

Object *objectCreate3(const std::string &type, const std::string &name, Object *parent) {
    return Engine::objectCreate(type, name, parent);
}

Actor *composeActor(const std::string &name, const std::string &component, Object *parent) {
    return Engine::composeActor(name, component, parent);
}

Object *loadResource(const std::string &name) {
    return Engine::loadResource(name);
}

void unloadResource(const std::string &name) {
    Engine::unloadResource(name);
}

void registerEngine(asIScriptEngine *engine) {
    engine->SetDefaultNamespace("Engine");

    engine->RegisterGlobalFunction("Object @objectCreate(const std::string &in)", asFUNCTION(objectCreate1), asCALL_CDECL);
    engine->RegisterGlobalFunction("Object @objectCreate(const std::string &in, const std::string &in)", asFUNCTION(objectCreate2), asCALL_CDECL);
    engine->RegisterGlobalFunction("Object @objectCreate(const std::string &in, const std::string &in, Object &in)", asFUNCTION(objectCreate3), asCALL_CDECL);

    engine->RegisterGlobalFunction("Actor @composeActor(const std::string &in, const std::string &in, Object &in)", asFUNCTION(composeActor), asCALL_CDECL);

    engine->RegisterGlobalFunction("Object @loadResource(const std::string &in)", asFUNCTION(loadResource), asCALL_CDECL);
    engine->RegisterGlobalFunction("void unloadResource(const std::string &in)", asFUNCTION(unloadResource), asCALL_CDECL);

    engine->RegisterGlobalFunction("Scene @loadScene(const std::string &in, bool)", asFUNCTION(Engine::loadScene), asCALL_CDECL);
    engine->RegisterGlobalFunction("void unloadScene(Scene &)", asFUNCTION(Engine::unloadScene), asCALL_CDECL);

    engine->SetDefaultNamespace("");
}

void registerCore(asIScriptEngine *engine) {
    engine->RegisterGlobalFunction("void debug(const std::string &in)", asFUNCTION(debugFunction), asCALL_CDECL);
    engine->RegisterGlobalFunction("void info(const std::string &in)", asFUNCTION(infoFunction), asCALL_CDECL);
    engine->RegisterGlobalFunction("void warning(const std::string &in)", asFUNCTION(warningFunction), asCALL_CDECL);
    engine->RegisterGlobalFunction("void error(const std::string &in)", asFUNCTION(errorFunction), asCALL_CDECL);
    engine->RegisterGlobalFunction("void critical(const std::string &in)", asFUNCTION(criticalFunction), asCALL_CDECL);

    registerObject(engine);
}
