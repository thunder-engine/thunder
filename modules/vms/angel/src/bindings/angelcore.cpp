#include "bindings/angelbindings.h"

#include <angelscript.h>
#include <autowrapper/aswrappedcall.h>

#include <engine.h>
#include <log.h>

#include <components/actor.h>

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

void wrap1Generic(asIScriptGeneric *gen) {

}

void registerObject(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("Object", 0, asOBJ_REF | asOBJ_NOCOUNT);

    engine->RegisterGlobalFunction("bool _connect(Object @, const string &in, Object @, const string &in)",
                                   generic ? asFUNCTION(wrap1Generic) : asFUNCTION(connect),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);
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

void registerEngine(asIScriptEngine *engine, bool generic) {
    engine->SetDefaultNamespace("Engine");

    engine->RegisterGlobalFunction("Object @objectCreate(const string &in)",
                                   generic ? WRAP_FN(objectCreate1) : asFUNCTION(objectCreate1),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Object @objectCreate(const string &in, const string &in)",
                                   generic ? WRAP_FN(objectCreate2) : asFUNCTION(objectCreate2),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Object @objectCreate(const string &in, const string &in, Object &in)",
                                   generic ? WRAP_FN(objectCreate3) : asFUNCTION(objectCreate3),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Actor @composeActor(const string &in, const string &in, Object &in)",
                                   generic ? WRAP_FN(composeActor) : asFUNCTION(composeActor),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Object @loadResource(const string &in)",
                                   generic ? WRAP_FN(loadResource) : asFUNCTION(loadResource),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void unloadResource(const string &in)",
                                   generic ? WRAP_FN(unloadResource) : asFUNCTION(unloadResource),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Scene @loadScene(const string &in, bool)",
                                   generic ? WRAP_FN(Engine::loadScene) : asFUNCTION(Engine::loadScene),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void unloadScene(Scene &)",
                                   generic ? WRAP_FN(Engine::unloadScene) : asFUNCTION(Engine::unloadScene),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->SetDefaultNamespace("");
}

void registerCore(asIScriptEngine *engine, bool generic) {
    engine->RegisterGlobalFunction("void debug(const string &in)",
                                   generic ? WRAP_FN(debugFunction) : asFUNCTION(debugFunction),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void info(const string &in)",
                                   generic ? WRAP_FN(infoFunction) : asFUNCTION(infoFunction),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void warning(const string &in)",
                                   generic ? WRAP_FN(warningFunction) : asFUNCTION(warningFunction),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void error(const string &in)",
                                   generic ? WRAP_FN(errorFunction) : asFUNCTION(errorFunction),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void critical(const string &in)",
                                   generic ? WRAP_FN(criticalFunction) : asFUNCTION(criticalFunction),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    registerObject(engine, generic);
}
