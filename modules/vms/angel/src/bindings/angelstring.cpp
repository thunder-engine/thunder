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
                                 generic ? WRAP_MFN_PR(TString, operator+, (const TString&) const, TString) : asMETHODPR(TString, operator+, (const TString&) const, TString),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
    engine->RegisterObjectMethod("TString", "TString &opAdd(const string &in)",
                                 generic ? WRAP_MFN_PR(TString, operator+, (const std::string&) const, TString) : asMETHODPR(TString, operator+, (const std::string&) const, TString),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);


}
