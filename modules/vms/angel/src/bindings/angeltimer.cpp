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

#include <timer.h>
#include <log.h>

void registerTimer(asIScriptEngine *engine, bool generic) {
    engine->SetDefaultNamespace("Timer");

    engine->RegisterGlobalFunction("float time()",
                                   generic ? WRAP_FN(Timer::time) : asFUNCTION(Timer::time),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("float deltaTime()",
                                   generic ? WRAP_FN(Timer::deltaTime): asFUNCTION(Timer::deltaTime),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("float scale()",
                                   generic ? WRAP_FN(Timer::scale) : asFUNCTION(Timer::scale),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("void setScale(float)",
                                   generic ? WRAP_FN(Timer::setScale) : asFUNCTION(Timer::setScale),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->SetDefaultNamespace("");
}
