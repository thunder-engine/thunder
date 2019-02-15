#include "bindings/angelcore.h"

#include <angelscript.h>

#include "engine.h"

void registerObject(asIScriptEngine *engine) {
    engine->RegisterObjectType("Object", 0, asOBJ_REF | asOBJ_NOCOUNT);
}

void registerEngine(asIScriptEngine *engine) {
    //engine->RegisterGlobalFunction("T @objectCreate<T>(const string &in, Object @&in)", asFUNCTION(Engine::objectCreate), asCALL_CDECL);

    engine->RegisterGlobalFunction("Object @objectCreate(const string &in, const string &in, Object &in)", asFUNCTION(Engine::objectCreateImpl), asCALL_CDECL);
    engine->RegisterGlobalFunction("Object @loadResource(const string &in)", asFUNCTION(Engine::loadResourceImpl), asCALL_CDECL);

    //r = engine->RegisterObjectBehaviour("myValueTemplate<T>", asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTIONPR(myValueTemplConstructor, (asITypeInfo*, void*), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
}

void registerCore(asIScriptEngine *engine) {
    registerObject(engine);

    registerEngine(engine);
}
