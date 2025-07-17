#include "bindings/angelbindings.h"

#include <angelscript.h>
#include <autowrapper/aswrappedcall.h>

#include <astring.h>

static void newString(TString *dest) {
    new (dest) TString();
}

static void deleteString(TString *dest) {
    dest->~TString();
}

static void newStdString(const std::string &str, TString *dest) {
    new (dest) TString(str);
}

void registerString(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("TString", sizeof(TString), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("TString", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_OBJ_LAST(newString) : asFUNCTION(newString),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("TString", asBEHAVE_CONSTRUCT, "void f(const string &in)",
                                    generic ? WRAP_OBJ_LAST(newStdString) : asFUNCTION(newStdString),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("TString", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_OBJ_LAST(deleteString) : asFUNCTION(deleteString),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("TString", "TString &opAssign(const TString &in)",
                                 generic ? WRAP_MFN_PR(TString, operator=, (const TString&), TString&) : asMETHODPR(TString, operator=, (const TString&), TString&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("TString", "bool opEquals(const TString &in)",
                                 generic ? WRAP_MFN(TString, operator==) : asMETHOD(TString, operator==),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("TString", "bool opCmp(const TString &in)",
                                 generic ? WRAP_MFN(TString, operator<) : asMETHOD(TString, operator<),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("TString", "TString &opAddAssign(const TString &in)",
                                 generic ? WRAP_MFN_PR(TString, operator+=, (const TString&), TString&) : asMETHODPR(TString, operator+=, (const TString&), TString&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
    engine->RegisterObjectMethod("TString", "TString &opAddAssign(const string &in)",
                                 generic ? WRAP_MFN_PR(TString, operator+=, (const std::string&), TString&) : asMETHODPR(TString, operator+=, (const std::string&), TString&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("TString", "TString &opAdd(const TString &in)",
                                 generic ? WRAP_MFN_PR(TString, operator+, (const TString&), TString) : asMETHODPR(TString, operator+, (const TString&), TString),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
    engine->RegisterObjectMethod("TString", "TString &opAdd(const string &in)",
                                 generic ? WRAP_MFN_PR(TString, operator+, (const std::string&), TString) : asMETHODPR(TString, operator+, (const std::string&), TString),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);


}
