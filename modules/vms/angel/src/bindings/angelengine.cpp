#include "bindings/angelbindings.h"

#include <angelscript.h>
#include <autowrapper/aswrappedcall.h>

#include <engine.h>

Object *objectCreate1(const TString &type) {
    return Engine::objectCreate(type);
}

Object *objectCreate2(const TString &type, const TString &name) {
    return Engine::objectCreate(type, name);
}

Object *objectCreate3(const TString &type, const TString &name, Object *parent) {
    return Engine::objectCreate(type, name, parent);
}

Actor *composeActor(const TString &name, const TString &component, Object *parent) {
    return Engine::composeActor(name, component, parent);
}

Resource *loadResource(const TString &name) {
    return Engine::loadResource(name);
}

void unloadResource(const TString &name) {
    Engine::unloadResource(name);
}

void registerEngine(asIScriptEngine *engine, bool generic) {
    engine->SetDefaultNamespace("Engine");

    engine->RegisterGlobalFunction("Object @objectCreate(const TString &in)",
                                   generic ? WRAP_FN(objectCreate1) : asFUNCTION(objectCreate1),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Object @objectCreate(const TString &in, const TString &in)",
                                   generic ? WRAP_FN(objectCreate2) : asFUNCTION(objectCreate2),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Object @objectCreate(const TString &in, const TString &in, Object &in)",
                                   generic ? WRAP_FN(objectCreate3) : asFUNCTION(objectCreate3),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Actor @composeActor(const TString &in, const TString &in, Object &in)",
                                   generic ? WRAP_FN(composeActor) : asFUNCTION(composeActor),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Resource @loadResource(const TString &in)",
                                   generic ? WRAP_FN(loadResource) : asFUNCTION(loadResource),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void unloadResource(const TString &in)",
                                   generic ? WRAP_FN(unloadResource) : asFUNCTION(unloadResource),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("Scene @loadScene(const TString &in, bool)",
                                   generic ? WRAP_FN(Engine::loadScene) : asFUNCTION(Engine::loadScene),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void unloadScene(Scene &)",
                                   generic ? WRAP_FN(Engine::unloadScene) : asFUNCTION(Engine::unloadScene),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->SetDefaultNamespace("");
}
