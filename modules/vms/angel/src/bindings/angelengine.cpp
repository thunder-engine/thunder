/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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

    engine->RegisterGlobalFunction("World &world()",
                                   generic ? WRAP_FN(Engine::world) : asFUNCTION(Engine::world),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->SetDefaultNamespace("");
}
