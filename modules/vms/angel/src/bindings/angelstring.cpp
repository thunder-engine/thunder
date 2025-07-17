#include "bindings/angelbindings.h"

#include <angelscript.h>
#include <autowrapper/aswrappedcall.h>

#include <astring.h>

using namespace next;

static void newString(String *dest) {
    new (dest) String();
}

static void deleteString(String *dest) {
    dest->~String();
}

static void newStdString(const std::string &str, String *dest) {
    new (dest) String(str);
}

void registerString(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("String", sizeof(String), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("String", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_OBJ_LAST(newString) : asFUNCTION(newString),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("String", asBEHAVE_CONSTRUCT, "void f(const string &in)",
                                    generic ? WRAP_OBJ_LAST(newStdString) : asFUNCTION(newStdString),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("String", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_OBJ_LAST(deleteString) : asFUNCTION(deleteString),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("String", "String &opAssign(const String &in)",
                                 generic ? WRAP_MFN_PR(String, operator=, (const String&), String&) : asMETHODPR(String, operator=, (const String&), String&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("String", "bool opEquals(const String &in)",
                                 generic ? WRAP_MFN(String, operator==) : asMETHOD(String, operator==),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("String", "bool opCmp(const String &in)",
                                 generic ? WRAP_MFN(String, operator<) : asMETHOD(String, operator<),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("String", "String &opAddAssign(const String &in)",
                                 generic ? WRAP_MFN_PR(String, operator+=, (const String&), String&) : asMETHODPR(String, operator+=, (const String&), String&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
    engine->RegisterObjectMethod("String", "String &opAddAssign(const string &in)",
                                 generic ? WRAP_MFN_PR(String, operator+=, (const std::string&), String&) : asMETHODPR(String, operator+=, (const std::string&), String&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("String", "String &opAdd(const String &in)",
                                 generic ? WRAP_MFN_PR(String, operator+, (const String&), String) : asMETHODPR(String, operator+, (const String&), String),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
    engine->RegisterObjectMethod("String", "String &opAdd(const string &in)",
                                 generic ? WRAP_MFN_PR(String, operator+, (const std::string&), String) : asMETHODPR(String, operator+, (const std::string&), String),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);


}
