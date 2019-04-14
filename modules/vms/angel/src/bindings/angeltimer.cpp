#include "bindings/angelbindings.h"

#include <angelscript.h>

#include <timer.h>

void registerTimer(asIScriptEngine *engine) {
    engine->SetDefaultNamespace("Timer");

    engine->RegisterGlobalFunction("float deltaTime()", asFUNCTION(Timer::deltaTime), asCALL_CDECL);
    engine->RegisterGlobalFunction("float scale()", asFUNCTION(Timer::scale), asCALL_CDECL);
    engine->RegisterGlobalFunction("void setScale(float)", asFUNCTION(Timer::setScale), asCALL_CDECL);

    engine->SetDefaultNamespace("");
}
