#include "bindings/angelbindings.h"

#include <angelscript.h>
#include <autowrapper/aswrappedcall.h>

#include <log.h>

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

void registerLog(asIScriptEngine *engine, bool generic) {
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
}
