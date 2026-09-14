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

#include <object.h>

bool connect(Object *sender, const std::string &signal, Object *receiver, const std::string &slot) {
    return Object::connect(sender, signal.c_str(), receiver, slot.c_str());
}

void registerObject(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("Object", 0, asOBJ_REF | asOBJ_NOCOUNT);

    engine->RegisterGlobalFunction("bool _connect(Object @, const string &in, Object @, const string &in)",
                                   generic ? WRAP_FN(connect) : asFUNCTION(connect),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);
}
