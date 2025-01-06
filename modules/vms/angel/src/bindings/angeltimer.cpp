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
