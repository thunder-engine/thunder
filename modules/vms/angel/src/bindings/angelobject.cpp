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
