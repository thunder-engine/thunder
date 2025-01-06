#include "bindings/angelbindings.h"

#include <angelscript.h>
#include <autowrapper/aswrappedcall.h>

#include <amath.h>

static void vec2(Vector2 *dest) {
    new (dest) Vector2();
}

static void deleteVec2(Vector2 *dest) {
    dest->~Vector2();
}

static void new2Float2(float x, float y, Vector2 *dest) {
    new (dest) Vector2(x, y);
}

static void new1Float2(float value, Vector2 *dest) {
    new (dest) Vector2(value);
}

static float vec2GetX(Vector2 *dest) {
    return dest->x;
}

static void vec2SetX(float value, Vector2 *dest) {
    dest->x = value;
}

static float vec2GetY(Vector2 *dest) {
    return dest->y;
}

static void vec2SetY(float value, Vector2 *dest) {
    dest->y = value;
}

void registerVector2(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("Vector2", sizeof(Vector2), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_CON(Vector2, ()) : asFUNCTION(vec2),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(float)",
                                    generic ? WRAP_CON(Vector2, (float)) : asFUNCTION(new1Float2),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(float, float)",
                                    generic ? WRAP_CON(Vector2, (float, float)) : asFUNCTION(new2Float2),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector2", asBEHAVE_DESTRUCT,  "void f()",
                                    generic ? WRAP_DES(Vector2) : asFUNCTION(deleteVec2),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector2", "Vector2 &opAssign(const Vector2 &in)",
                                 generic ? WRAP_MFN_PR(Vector2, operator=, (const Vector2&), Vector2&) : asMETHODPR(Vector2, operator=, (const Vector2&), Vector2&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "bool opEquals(const Vector2 &in)",
                                 generic ? WRAP_MFN(Vector2, operator==) : asMETHOD(Vector2, operator==),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "bool opCmp(const Vector2 &in)",
                                 generic ? WRAP_MFN(Vector2, operator<) : asMETHOD(Vector2, operator<),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 opMul(float)",
                                 generic ? WRAP_MFN_PR(Vector2, operator*, (areal) const, Vector2) : asMETHODPR(Vector2, operator*, (areal) const, Vector2),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 opMul(const Vector2 &in)",
                                 generic ? WRAP_MFN_PR(Vector2, operator*, (const Vector2 &) const, Vector2) : asMETHODPR(Vector2, operator*, (const Vector2 &) const, Vector2),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 opDiv(float)",
                                 generic ? WRAP_MFN(Vector2, operator/) : asMETHOD(Vector2, operator/),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 opAdd(const Vector2 &in)",
                                 generic ? WRAP_MFN(Vector2, operator+) : asMETHOD(Vector2, operator+),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 opNeg()",
                                 generic ? WRAP_MFN_PR(Vector2, operator-, () const, Vector2) : asMETHODPR(Vector2, operator-, () const, Vector2),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 opSub(const Vector2 &in)",
                                 generic ? WRAP_MFN_PR(Vector2, operator-, (const Vector2 &) const, Vector2) : asMETHODPR(Vector2, operator-, (const Vector2 &) const, Vector2),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 &opMulAssign(float)",
                                 generic ? WRAP_MFN(Vector2, operator*=) : asMETHOD(Vector2, operator*=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 &opDivAssign(float)",
                                 generic ? WRAP_MFN(Vector2, operator/=) : asMETHOD(Vector2, operator/=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 &opAddAssign(const Vector2 &in)",
                                 generic ? WRAP_MFN(Vector2, operator+=) : asMETHOD(Vector2, operator+=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 &opSubAssign(const Vector2 &in)",
                                 generic ? WRAP_MFN(Vector2, operator-=) : asMETHOD(Vector2, operator-=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "float opIndex(int)",
                                 generic ? WRAP_MFN_PR(Vector2, operator[], (int i) const, float) : asMETHODPR(Vector2, operator[], (int i) const, float),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "float length()",
                                 generic ? WRAP_MFN(Vector2, length) : asMETHOD(Vector2, length),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "float sqrLength()",
                                 generic ? WRAP_MFN(Vector2, sqrLength) : asMETHOD(Vector2, sqrLength),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "float normalize()",
                                 generic ? WRAP_MFN(Vector2, normalize) : asMETHOD(Vector2, normalize),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 &cross(const Vector2 &in)",
                                 generic ? WRAP_MFN(Vector2, cross) : asMETHOD(Vector2, cross),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "float dot(const Vector2 &in)",
                                 generic ? WRAP_MFN(Vector2, dot) : asMETHOD(Vector2, dot),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "float get_x() property",
                                 generic ? WRAP_FN(vec2GetX) : asFUNCTION(vec2GetX),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector2", "void set_x(float) property",
                                 generic ? WRAP_FN(vec2SetX) : asFUNCTION(vec2SetX),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector2", "float get_y() property",
                                 generic ? WRAP_FN(vec2GetY) : asFUNCTION(vec2GetY),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector2", "void set_y(float) property",
                                 generic ? WRAP_FN(vec2SetY) : asFUNCTION(vec2SetY),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);
}

static void vec3(Vector3 *dest) {
    new (dest) Vector3();
}

static void deleteVec3(Vector3 *dest) {
    dest->~Vector3();
}

static void new3Float3(float x, float y, float z, Vector3 *dest) {
    new (dest) Vector3(x, y, z);
}

static void new1Float3(float value, Vector3 *dest) {
    new (dest) Vector3(value);
}

static void newVec2Float3(const Vector2 &in, float z, Vector3 *dest) {
    new (dest) Vector3(in, z);
}

static float vec3GetX(Vector3 *dest) {
    return dest->x;
}

static void vec3SetX(float value, Vector3 *dest) {
    dest->x = value;
}

static float vec3GetY(Vector3 *dest) {
    return dest->y;
}

static void vec3SetY(float value, Vector3 *dest) {
    dest->y = value;
}

static float vec3GetZ(Vector3 *dest) {
    return dest->z;
}

static void vec3SetZ(float value, Vector3 *dest) {
    dest->z = value;
}

void registerVector3(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("Vector3", sizeof(Vector3), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_CON(Vector3, ()) : asFUNCTION(vec3),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(float)",
                                    generic ? WRAP_CON(Vector3, (float)) : asFUNCTION(new1Float3),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(float, float, float)",
                                    generic ? WRAP_CON(Vector3, (float, float, float)) : asFUNCTION(new3Float3),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(const Vector2 &in, float)",
                                    generic ? WRAP_CON(Vector3, (const Vector2 &, float)) : asFUNCTION(newVec2Float3),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector3", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_DES(Vector3) : asFUNCTION(deleteVec3),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector3", "Vector3 &opAssign(const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(Vector3, operator=, (const Vector3&), Vector3&) : asMETHODPR(Vector3, operator=, (const Vector3&), Vector3&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "bool opEquals(const Vector3 &in)",
                                 generic ? WRAP_MFN(Vector3, operator==) : asMETHOD(Vector3, operator==),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "bool opCmp(const Vector3 &in)",
                                 generic ? WRAP_MFN(Vector3, operator<) : asMETHOD(Vector3, operator<),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 opMul(float)",
                                 generic ? WRAP_MFN_PR(Vector3, operator*, (areal) const, Vector3) : asMETHODPR(Vector3, operator*, (areal) const, Vector3),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 opMul(const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(Vector3, operator*, (const Vector3 &) const, Vector3) : asMETHODPR(Vector3, operator*, (const Vector3 &) const, Vector3),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 opDiv(float)",
                                 generic ? WRAP_MFN(Vector3, operator/) : asMETHOD(Vector3, operator/),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 opAdd(const Vector3 &in)",
                                 generic ? WRAP_MFN(Vector3, operator+) : asMETHOD(Vector3, operator+),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 opNeg()",
                                 generic ? WRAP_MFN_PR(Vector3, operator-, () const, Vector3) : asMETHODPR(Vector3, operator-, () const, Vector3),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 opSub(const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(Vector3, operator-, (const Vector3 &) const, Vector3) : asMETHODPR(Vector3, operator-, (const Vector3 &) const, Vector3),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 &opMulAssign(float)",
                                 generic ? WRAP_MFN(Vector3, operator*=) : asMETHOD(Vector3, operator*=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 &opDivAssign(float)",
                                 generic ? WRAP_MFN(Vector3, operator/=) : asMETHOD(Vector3, operator/=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 &opAddAssign(const Vector3 &in)",
                                 generic ? WRAP_MFN(Vector3, operator+=) : asMETHOD(Vector3, operator+=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 &opSubAssign(const Vector3 &in)",
                                 generic ? WRAP_MFN(Vector3, operator-=) : asMETHOD(Vector3, operator-=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float opIndex(int)",
                                 generic ? WRAP_MFN_PR(Vector3, operator[], (int i) const, float) : asMETHODPR(Vector3, operator[], (int i) const, float),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float length()",
                                 generic ? WRAP_MFN(Vector3, length) : asMETHOD(Vector3, length),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float sqrLength()",
                                 generic ? WRAP_MFN(Vector3, sqrLength) : asMETHOD(Vector3, sqrLength),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float normalize()",
                                 generic ? WRAP_MFN(Vector3, normalize) : asMETHOD(Vector3, normalize),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 &cross(const Vector3 &in)",
                                 generic ? WRAP_MFN(Vector3, cross) : asMETHOD(Vector3, cross),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float dot(const Vector3 &in)",
                                 generic ? WRAP_MFN(Vector3, dot) : asMETHOD(Vector3, dot),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float angle(const Vector3 &in)",
                                 generic ? WRAP_MFN(Vector3, angle) : asMETHOD(Vector3, angle),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float signedAngle(const Vector3 &in, const Vector3 &in)",
                                 generic ? WRAP_MFN(Vector3, signedAngle) : asMETHOD(Vector3, signedAngle),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float get_x() property",
                                 generic ? WRAP_FN(vec3GetX) : asFUNCTION(vec3GetX),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector3", "void set_x(float) property",
                                 generic ? WRAP_FN(vec3SetX) : asFUNCTION(vec3SetX),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector3", "float get_y() property",
                                 generic ? WRAP_FN(vec3GetY) : asFUNCTION(vec3GetY),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector3", "void set_y(float) property",
                                 generic ? WRAP_FN(vec3SetY) : asFUNCTION(vec3SetY),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector3", "float get_z() property",
                                 generic ? WRAP_FN(vec3GetZ) : asFUNCTION(vec3GetZ),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector3", "void set_z(float) property",
                                 generic ? WRAP_FN(vec3SetZ) : asFUNCTION(vec3SetZ),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);
}

static void vec4(Vector4 *dest) {
    new (dest) Vector4();
}

static void deleteVec4(Vector4 *dest) {
    dest->~Vector4();
}

static void new4Float4(float x, float y, float z, float w, Vector4 *dest) {
    new (dest) Vector4(x, y, z, w);
}

static void new1Float4(float value, Vector4 *dest) {
    new (dest) Vector4(value);
}

static void newVec2Float4(const Vector2 &in, float z, float w, Vector4 *dest) {
    new (dest) Vector4(in, z, w);
}

static void newVec3Float4(const Vector3 &in, float w, Vector4 *dest) {
    new (dest) Vector4(in, w);
}

static float vec4GetX(Vector4 *dest) {
    return dest->x;
}

static void vec4SetX(float value, Vector4 *dest) {
    dest->x = value;
}

static float vec4GetY(Vector4 *dest) {
    return dest->y;
}

static void vec4SetY(float value, Vector4 *dest) {
    dest->y = value;
}

static float vec4GetZ(Vector4 *dest) {
    return dest->z;
}

static void vec4SetZ(float value, Vector4 *dest) {
    dest->z = value;
}

static float vec4GetW(Vector4 *dest) {
    return dest->w;
}

static void vec4SetW(float value, Vector4 *dest) {
    dest->w = value;
}

void registerVector4(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("Vector4", sizeof(Vector4), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_CON(Vector4, ()) : asFUNCTION(vec4),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f(float)",
                                    generic ? WRAP_CON(Vector4, (float)) : asFUNCTION(new1Float4),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)",
                                    generic ? WRAP_CON(Vector4, (float, float, float, float)) : asFUNCTION(new4Float4),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f(const Vector2 &in, float, float)",
                                    generic ? WRAP_CON(Vector4, (const Vector2 &, float, float)) : asFUNCTION(newVec2Float4),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, float)",
                                    generic ? WRAP_CON(Vector4, (const Vector3 &, float)) : asFUNCTION(newVec3Float4),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_DES(Vector4) : asFUNCTION(deleteVec4),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector4", "Vector4 &opAssign(const Vector4 &in)",
                                 generic ? WRAP_MFN_PR(Vector4, operator=, (const Vector4&), Vector4&) : asMETHODPR(Vector4, operator=, (const Vector4&), Vector4&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "bool opEquals(const Vector4 &in)",
                                 generic ? WRAP_MFN(Vector4, operator==) : asMETHOD(Vector4, operator==),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "bool opCmp(const Vector4 &in)",
                                 generic ? WRAP_MFN(Vector4, operator<) : asMETHOD(Vector4, operator<),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 opMul(float)",
                                 generic ? WRAP_MFN_PR(Vector4, operator*, (areal) const, Vector4) : asMETHODPR(Vector4, operator*, (areal) const, Vector4),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 opMul(const Vector4 &in)",
                                 generic ? WRAP_MFN_PR(Vector4, operator*, (const Vector4 &) const, Vector4) : asMETHODPR(Vector4, operator*, (const Vector4 &) const, Vector4),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 opDiv(float)",
                                 generic ? WRAP_MFN(Vector4, operator/) : asMETHOD(Vector4, operator/),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 opAdd(const Vector4 &in)",
                                 generic ? WRAP_MFN(Vector4, operator+) : asMETHOD(Vector4, operator+),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 opNeg()",
                                 generic ? WRAP_MFN_PR(Vector4, operator-, () const, Vector4) : asMETHODPR(Vector4, operator-, () const, Vector4),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 opSub(const Vector4 &in)",
                                 generic ? WRAP_MFN_PR(Vector4, operator-, (const Vector4 &) const, Vector4) : asMETHODPR(Vector4, operator-, (const Vector4 &) const, Vector4),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 &opMulAssign(float)",
                                 generic ? WRAP_MFN(Vector4, operator*=) : asMETHOD(Vector4, operator*=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 &opDivAssign(float)",
                                 generic ? WRAP_MFN(Vector4, operator/=) : asMETHOD(Vector4, operator/=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 &opAddAssign(const Vector4 &in)",
                                 generic ? WRAP_MFN(Vector4, operator+=) : asMETHOD(Vector4, operator+=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 &opSubAssign(const Vector4 &in)",
                                 generic ? WRAP_MFN(Vector4, operator-=) : asMETHOD(Vector4, operator-=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "float opIndex(int)",
                                 generic ? WRAP_MFN_PR(Vector4, operator[], (int i) const, float) : asMETHODPR(Vector4, operator[], (int i) const, float),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "float length()",
                                 generic ? WRAP_MFN(Vector4, length) : asMETHOD(Vector4, length),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "float sqrLength()",
                                 generic ? WRAP_MFN(Vector4, sqrLength) : asMETHOD(Vector4, sqrLength),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "float normalize()",
                                 generic ? WRAP_MFN(Vector4, normalize) : asMETHOD(Vector4, normalize),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "float dot(const Vector4 &in)",
                                 generic ? WRAP_MFN(Vector4, dot) : asMETHOD(Vector4, dot),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "float get_x() property",
                                 generic ? WRAP_FN(vec4GetX) : asFUNCTION(vec4GetX),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector4", "void set_x(float) property",
                                 generic ? WRAP_FN(vec4SetX) : asFUNCTION(vec4SetX),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector4", "float get_y() property",
                                 generic ? WRAP_FN(vec4GetY) : asFUNCTION(vec4GetY),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector4", "void set_y(float) property",
                                 generic ? WRAP_FN(vec4SetY) : asFUNCTION(vec4SetY),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector4", "float get_z() property",
                                 generic ? WRAP_FN(vec4GetZ) : asFUNCTION(vec4GetZ),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector4", "void set_z(float) property",
                                 generic ? WRAP_FN(vec4SetZ) : asFUNCTION(vec4SetZ),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector4", "float get_w() property",
                                 generic ? WRAP_FN(vec4GetW) : asFUNCTION(vec4GetW),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector4", "void set_w(float) property",
                                 generic ? WRAP_FN(vec4SetW) : asFUNCTION(vec4SetW),
                                 generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);
}

static void mat3(Matrix3 *dest) {
    new (dest) Matrix3();
}

static void deleteMat3(Matrix3 *dest) {
    dest->~Matrix3();
}

void registerMatrix3(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("Matrix3", sizeof(Matrix3), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Matrix3", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_CON(Matrix3, ()) : asFUNCTION(mat3),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Matrix3", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_DES(Matrix3) : asFUNCTION(deleteMat3),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 &opAssign(const Matrix3 &in)",
                                 generic ? WRAP_MFN_PR(Matrix3, operator=, (const Matrix3&), Matrix3&) : asMETHODPR(Matrix3, operator=, (const Matrix3&), Matrix3&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "bool opEquals(const Matrix3 &in)",
                                 generic ? WRAP_MFN(Matrix3, operator==) : asMETHOD(Matrix3, operator==),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 opMul(float)",
                                 generic ? WRAP_MFN_PR(Matrix3, operator*, (areal) const, Matrix3) : asMETHODPR(Matrix3, operator*, (areal) const, Matrix3),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Vector3 opMul(const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(Matrix3, operator*, (const Vector3 &) const, Vector3) : asMETHODPR(Matrix3, operator*, (const Vector3 &) const, Vector3),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Vector4 opMul(const Vector4 &in)",
                                 generic ? WRAP_MFN_PR(Matrix3, operator*, (const Vector4 &) const, Vector4) : asMETHODPR(Matrix3, operator*, (const Vector4 &) const, Vector4),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 opMul(const Matrix3 &in)",
                                 generic ? WRAP_MFN_PR(Matrix3, operator*, (const Matrix3 &) const, Matrix3) : asMETHODPR(Matrix3, operator*, (const Matrix3 &) const, Matrix3),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 opAdd(const Matrix3 &in)",
                                 generic ? WRAP_MFN(Matrix3, operator+) : asMETHOD(Matrix3, operator+),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 opSub(const Matrix3 &in)",
                                 generic ? WRAP_MFN(Matrix3, operator-) : asMETHOD(Matrix3, operator-),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 &opMulAssign(float)",
                                 generic ? WRAP_MFN_PR(Matrix3, operator*=, (areal), Matrix3&) : asMETHODPR(Matrix3, operator*=, (areal), Matrix3&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 &opMulAssign(const Matrix3 &in)",
                                 generic ? WRAP_MFN_PR(Matrix3, operator*=, (const Matrix3 &), Matrix3&) : asMETHODPR(Matrix3, operator*=, (const Matrix3 &), Matrix3&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 &opAddAssign(const Matrix3 &in)",
                                 generic ? WRAP_MFN(Matrix3, operator+=) : asMETHOD(Matrix3, operator+=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 &opSubAssign(const Matrix3 &in)",
                                 generic ? WRAP_MFN(Matrix3, operator-=) : asMETHOD(Matrix3, operator-=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "float opIndex(int)",
                                 generic ? WRAP_MFN_PR(Matrix3, operator[], (int i) const, float) : asMETHODPR(Matrix3, operator[], (int i) const, float),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 transpose()",
                                 generic ? WRAP_MFN(Matrix3, transpose) : asMETHOD(Matrix3, transpose),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "float determinant()",
                                 generic ? WRAP_MFN(Matrix3, determinant) : asMETHOD(Matrix3, determinant),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 inverse()",
                                 generic ? WRAP_MFN(Matrix3, inverse) : asMETHOD(Matrix3, inverse),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "void zero()",
                                 generic ? WRAP_MFN(Matrix3, zero) : asMETHOD(Matrix3, zero),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "void identity()",
                                 generic ? WRAP_MFN(Matrix3, identity) : asMETHOD(Matrix3, identity),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "void rotate(const Vector3 &in, float angle)",
                                 generic ? WRAP_MFN_PR(Matrix3, rotate, (const Vector3 &, float), void) : asMETHODPR(Matrix3, rotate, (const Vector3 &, float), void),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "void rotate(const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(Matrix3, rotate, (const Vector3 &), void) : asMETHODPR(Matrix3, rotate, (const Vector3 &), void),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "void scale(const Vector3 &in)",
                                 generic ? WRAP_MFN(Matrix3, scale) : asMETHOD(Matrix3, scale),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "void orthonormalize()",
                                 generic ? WRAP_MFN(Matrix3, orthonormalize) : asMETHOD(Matrix3, orthonormalize),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
}

static void mat4(Matrix4 *dest) {
    new (dest) Matrix4();
}

static void deleteMat4(Matrix4 *dest) {
    dest->~Matrix4();
}

void registerMatrix4(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("Matrix4", sizeof(Matrix4), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Matrix4", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_CON(Matrix4, ()) : asFUNCTION(mat4),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Matrix4", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_DES(Matrix4) : asFUNCTION(deleteMat4),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 &opAssign(const Matrix4 &in)",
                                 generic ? WRAP_MFN_PR(Matrix4, operator=, (const Matrix4&), Matrix4&) : asMETHODPR(Matrix4, operator=, (const Matrix4&), Matrix4&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "bool opEquals(const Matrix4 &in)",
                                 generic ? WRAP_MFN(Matrix4, operator==) : asMETHOD(Matrix4, operator==),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 opMul(float)",
                                 generic ? WRAP_MFN_PR(Matrix4, operator*, (areal) const, Matrix4) : asMETHODPR(Matrix4, operator*, (areal) const, Matrix4),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Vector3 opMul(const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(Matrix4, operator*, (const Vector3 &) const, Vector3) : asMETHODPR(Matrix4, operator*, (const Vector3 &) const, Vector3),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Vector4 opMul(const Vector4 &in)",
                                 generic ? WRAP_MFN_PR(Matrix4, operator*, (const Vector4 &) const, Vector4) : asMETHODPR(Matrix4, operator*, (const Vector4 &) const, Vector4),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 opMul(const Matrix4 &in)",
                                 generic ? WRAP_MFN_PR(Matrix4, operator*, (const Matrix4 &) const, Matrix4) : asMETHODPR(Matrix4, operator*, (const Matrix4 &) const, Matrix4),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 opAdd(const Matrix4 &in)",
                                 generic ? WRAP_MFN(Matrix4, operator+) : asMETHOD(Matrix4, operator+),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 opSub(const Matrix4 &in)",
                                 generic ? WRAP_MFN(Matrix4, operator-) : asMETHOD(Matrix4, operator-),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 &opMulAssign(float)",
                                 generic ? WRAP_MFN_PR(Matrix4, operator*=, (areal), Matrix4&) : asMETHODPR(Matrix4, operator*=, (areal), Matrix4&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 &opMulAssign(const Matrix4 &in)",
                                 generic ? WRAP_MFN_PR(Matrix4, operator*=, (const Matrix4 &), Matrix4&) : asMETHODPR(Matrix4, operator*=, (const Matrix4 &), Matrix4&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 &opAddAssign(const Matrix4 &in)",
                                 generic ? WRAP_MFN(Matrix4, operator+=) : asMETHOD(Matrix4, operator+=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 &opSubAssign(const Matrix4 &in)",
                                 generic ? WRAP_MFN(Matrix4, operator-=) : asMETHOD(Matrix4, operator-=),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "float opIndex(int)",
                                 generic ? WRAP_MFN_PR(Matrix4, operator[], (int i) const, float) : asMETHODPR(Matrix4, operator[], (int i) const, float),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix3 rotation()",
                                 generic ? WRAP_MFN(Matrix4, rotation) : asMETHOD(Matrix4, rotation),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 transpose()",
                                 generic ? WRAP_MFN(Matrix4, transpose) : asMETHOD(Matrix4, transpose),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "float determinant()",
                                 generic ? WRAP_MFN(Matrix4, determinant) : asMETHOD(Matrix4, determinant),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 inverse()",
                                 generic ? WRAP_MFN(Matrix4, inverse) : asMETHOD(Matrix4, inverse),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void reflect(const Vector4 &in)",
                                 generic ? WRAP_MFN(Matrix4, reflect) : asMETHOD(Matrix4, reflect),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void direction(const Vector3 &in, const Vector3 &in)",
                                 generic ? WRAP_MFN(Matrix4, direction) : asMETHOD(Matrix4, direction),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Vector3 euler()",
                                 generic ? WRAP_MFN(Matrix4, euler) : asMETHOD(Matrix4, euler),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void zero()",
                                 generic ? WRAP_MFN(Matrix4, zero) : asMETHOD(Matrix4, zero),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void identity()",
                                 generic ? WRAP_MFN(Matrix4, identity) : asMETHOD(Matrix4, identity),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void rotate(const Vector3 &in, float)",
                                 generic ? WRAP_MFN_PR(Matrix4, rotate, (const Vector3 &, float), void) : asMETHODPR(Matrix4, rotate, (const Vector3 &, float), void),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void rotate(const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(Matrix4, rotate, (const Vector3 &), void) : asMETHODPR(Matrix4, rotate, (const Vector3 &), void),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void scale(const Vector3 &in)",
                                 generic ? WRAP_MFN(Matrix4, scale) : asMETHOD(Matrix4, scale),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void translate(const Vector3 &in)",
                                 generic ? WRAP_MFN(Matrix4, translate) : asMETHOD(Matrix4, translate),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
}

static void quat(Quaternion *dest) {
    new (dest) Quaternion();
}

static void deleteQuat(Quaternion *dest) {
    dest->~Quaternion();
}

static void quatAxis(const Vector3 &dir, areal angle, Quaternion *dest) {
    new (dest) Quaternion(dir, angle);
}

static void quatEuler(const Vector3 &euler, Quaternion *dest) {
    new (dest) Quaternion(euler);
}

static void quatMat(const Matrix3 &matrix, Quaternion *dest) {
    new (dest) Quaternion(matrix);
}

void registerQuaternion(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("Quaternion", sizeof(Quaternion), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_CON(Quaternion, ()) : asFUNCTION(quat),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, float)",
                                    generic ? WRAP_CON(Quaternion, (const Vector3 &, float)) : asFUNCTION(quatAxis),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in)",
                                    generic ? WRAP_CON(Quaternion, (const Vector3 &)) : asFUNCTION(quatEuler),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(const Matrix3 &in)",
                                    generic ? WRAP_CON(Quaternion, (const Matrix3 &)) : asFUNCTION(quatMat),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_DES(Quaternion) : asFUNCTION(deleteQuat),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Quaternion", "Quaternion &opAssign(const Quaternion &in)",
                                 generic ? WRAP_MFN_PR(Quaternion, operator=, (const Quaternion&), Quaternion&) : asMETHODPR(Quaternion, operator=, (const Quaternion&), Quaternion&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "bool opEquals(const Quaternion &in)",
                                 generic ? WRAP_MFN(Quaternion, operator==) : asMETHOD(Quaternion, operator==),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "Vector3 opMul(const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(Quaternion, operator*, (const Vector3 &) const, Vector3) : asMETHODPR(Quaternion, operator*, (const Vector3 &) const, Vector3),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "Quaternion opMul(const Vector4 &in)",
                                 generic ? WRAP_MFN_PR(Quaternion, operator*, (const Quaternion &) const, Quaternion) : asMETHODPR(Quaternion, operator*, (const Quaternion &) const, Quaternion),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "float opIndex(int)",
                                 generic ? WRAP_MFN_PR(Quaternion, operator[], (int i) const, float) : asMETHODPR(Quaternion, operator[], (int i) const, float),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "Quaternion inverse()",
                                 generic ? WRAP_MFN(Quaternion, inverse) : asMETHOD(Quaternion, inverse),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "void mix(const Quaternion &in, const Quaternion &in, float)",
                                 generic ? WRAP_MFN(Quaternion, mix) : asMETHOD(Quaternion, mix),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "Matrix3 toMatrix()",
                                 generic ? WRAP_MFN(Quaternion, toMatrix) : asMETHOD(Quaternion, toMatrix),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "Vector3 euler()",
                                 generic ? WRAP_MFN(Quaternion, euler) : asMETHOD(Quaternion, euler),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
}

static void aabb(AABBox *dest) {
    new (dest) AABBox();
}

static void deleteAABB(AABBox *dest) {
    dest->~AABBox();
}

static void aabbSize(const Vector3 &center, const Vector3 &size, AABBox *dest) {
    new (dest) AABBox(center, size);
}

void registerAABBox(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("AABBox", sizeof(AABBox), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("AABBox", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_CON(AABBox, ()) : asFUNCTION(aabb),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("AABBox", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, const Vector3 &in)",
                                    generic ? WRAP_CON(AABBox, (const Vector3 &, const Vector3 &)) : asFUNCTION(aabbSize),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("AABBox", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_DES(AABBox) : asFUNCTION(deleteAABB),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("AABBox", "AABBox &opAssign(const AABBox &in)",
                                 generic ? WRAP_MFN_PR(AABBox, operator=, (const AABBox&), AABBox&) : asMETHODPR(AABBox, operator=, (const AABBox&), AABBox&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("AABBox", "AABBox opMul(float)",
                                 generic ? WRAP_MFN_PR(AABBox, operator*, (areal) const, const AABBox) : asMETHODPR(AABBox, operator*, (areal) const, const AABBox),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("AABBox", "AABBox opMul(const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(AABBox, operator*, (const Vector3 &) const, const AABBox) : asMETHODPR(AABBox, operator*, (const Vector3 &) const, const AABBox),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("AABBox", "AABBox opMul(const Matrix4 &in)",
                                 generic ? WRAP_MFN_PR(AABBox, operator*, (const Matrix4 &) const, const AABBox) : asMETHODPR(AABBox, operator*, (const Matrix4 &) const, const AABBox),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    //engine->RegisterObjectMethod("AABBox", "bool intersect(const Vector3 &in, float)", asMETHOD(AABBox, intersect), asCALL_THISCALL);

    engine->RegisterObjectMethod("AABBox", "void box(Vector3 &out, Vector3 &out)",
                                 generic ? WRAP_MFN(AABBox, box) : asMETHOD(AABBox, box),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("AABBox", "void setBox(const Vector3 &in, const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(AABBox, setBox, (const Vector3 &, const Vector3 &), void) : asMETHODPR(AABBox, setBox, (const Vector3 &, const Vector3 &), void),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
}

static void obb(OBBox *dest) {
    new (dest) OBBox();
}

static void deleteOBB(OBBox *dest) {
    dest->~OBBox();
}

static void obbSize(const Vector3 &center, const Vector3 &size, OBBox *dest) {
    new (dest) OBBox(center, size);
}

static void obbSizeRot(const Vector3 &center, const Vector3 &size, OBBox *dest) {
    new (dest) OBBox(center, size);
}

void registerOBBox(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("OBBox", sizeof(OBBox), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("OBBox", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_CON(OBBox, ()) : asFUNCTION(obb),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("OBBox", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, const Vector3 &in)",
                                    generic ? WRAP_CON(OBBox, (const Vector3 &, const Vector3 &)) : asFUNCTION(obbSize),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("OBBox", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, const Vector3 &in, const Quaternion &in)",
                                    generic ? WRAP_CON(OBBox, (const Vector3 &, const Vector3 &, const Quaternion &)) : asFUNCTION(obbSizeRot),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("OBBox", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_DES(OBBox) : asFUNCTION(deleteOBB),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("OBBox", "OBBox &opAssign(const OBBox &in)",
                                 generic ? WRAP_MFN_PR(OBBox, operator=, (const OBBox&), OBBox&) : asMETHODPR(OBBox, operator=, (const OBBox&), OBBox&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("OBBox", "OBBox opMul(float)",
                                 generic ? WRAP_MFN_PR(OBBox, operator*, (areal) const, const OBBox) : asMETHODPR(OBBox, operator*, (areal) const, const OBBox),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("OBBox", "OBBox opMul(const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(OBBox, operator*, (const Vector3 &) const, const OBBox) : asMETHODPR(OBBox, operator*, (const Vector3 &) const, const OBBox),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("OBBox", "void box(const Vector3 &out, const Vector3 &out)",
                                 generic ? WRAP_MFN(OBBox, box) : asMETHOD(OBBox, box),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("OBBox", "void setBox(const Vector3 &in, const Vector3 &in)",
                                 generic ? WRAP_MFN(OBBox, setBox) : asMETHOD(OBBox, setBox),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
}

static void plane(Plane *dest) {
    new (dest) Plane();
}

static void planePoints(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, Plane *dest) {
    new (dest) Plane(v1, v2, v3);
}

static void deletePlane(Plane *dest) {
    dest->~Plane();
}

void registerPlane(asIScriptEngine *engine, bool generic) {
    engine->RegisterObjectType("Plane", sizeof(Plane), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Plane", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_CON(Plane, ()) : asFUNCTION(plane),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Plane", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, const Vector3 &in, const Vector3 &in)",
                                    generic ? WRAP_CON(Plane, (Vector3, Vector3, Vector3)) : asFUNCTION(planePoints),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Plane", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_DES(Plane) : asFUNCTION(deletePlane),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Plane", "Plane &opAssign(const Plane &in)",
                                 generic ? WRAP_MFN_PR(Plane, operator=, (const Plane&), Plane&) : asMETHODPR(Plane, operator=, (const Plane&), Plane&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Plane", "float sqrDistance(const Vector3 &in)",
                                 generic ? WRAP_MFN(Plane, sqrDistance) : asMETHOD(Plane, sqrDistance),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
}

static void hit(Ray *dest) {
    new (dest)Ray::Hit();
}

static void deleteHit(Ray::Hit *dest) {
    dest->~Hit();
}

static void ray(const Vector3 &position, const Vector3 &direction, Ray *dest) {
    new (dest) Ray(position, direction);
}

static void deleteRay(Ray *dest) {
    dest->~Ray();
}

void registerRay(asIScriptEngine *engine, bool generic) {
    engine->SetDefaultNamespace("Ray");

    engine->RegisterObjectType("Hit", sizeof(Ray::Hit), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Hit", asBEHAVE_CONSTRUCT, "void f()",
                                    generic ? WRAP_CON(Ray::Hit, ()) : asFUNCTION(hit),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Hit", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_DES(Ray::Hit) : asFUNCTION(deleteHit),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->SetDefaultNamespace("");

    engine->RegisterObjectType("Ray", sizeof(Ray), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Ray", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, const Vector3 &in)",
                                    generic ? WRAP_CON(Ray, (const Vector3 &, const Vector3 &)) : asFUNCTION(ray),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Ray", asBEHAVE_DESTRUCT, "void f()",
                                    generic ? WRAP_DES(Ray) : asFUNCTION(deleteRay),
                                    generic ? asCALL_GENERIC : asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Ray", "Ray &opAssign(const Ray &in)",
                                 generic ? WRAP_MFN_PR(Ray, operator=, (const Ray&), Ray&) : asMETHODPR(Ray, operator=, (const Ray&), Ray&),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Ray", "bool intersect(const Vector3 &in, float, Ray::Hit &out)",
                                 generic ? WRAP_MFN_PR(Ray, intersect, (const Vector3&, areal, Ray::Hit *), bool) : asMETHODPR(Ray, intersect, (const Vector3&, areal, Ray::Hit *), bool),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Ray", "bool intersect(const Plane &in, Ray::Hit &out, bool)",
                                 generic ? WRAP_MFN_PR(Ray, intersect, (const Plane &, Ray::Hit *, bool), bool) : asMETHODPR(Ray, intersect, (const Plane &, Ray::Hit *, bool), bool),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Ray", "bool intersect(const AABBox &in, Ray::Hit &out)",
                                 generic ? WRAP_MFN_PR(Ray, intersect, (const AABBox&, Ray::Hit *), bool) : asMETHODPR(Ray, intersect, (const AABBox&, Ray::Hit *), bool),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    //engine->RegisterObjectMethod("Ray", "bool intersect(const Vector3 &in, const Vector3 &in, const Vector3 &in, Ray::Hit &out, bool)", asMETHODPR(Ray, intersect, (const Vector3 &, const Vector3 &, const Vector3 &, Ray::Hit *, bool), bool), asCALL_THISCALL);

    engine->RegisterObjectMethod("Ray", "Ray reflect(const Vector3 &in, const Vector3 &in)",
                                 generic ? WRAP_MFN_PR(Ray, reflect, (const Vector3 &, const Vector3 &), Ray) : asMETHOD(Ray, reflect),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Ray", "Ray refract(const Vector3 &in, const Vector3 &in, float)",
                                 generic ? WRAP_MFN(Ray, refract) : asMETHOD(Ray, refract),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);

    engine->RegisterObjectMethod("Ray", "Ray diffuse(const Vector3 &in, const Vector3 &in, float, float)",
                                 generic ? WRAP_MFN(Ray, diffuse) : asMETHOD(Ray, diffuse),
                                 generic ? asCALL_GENERIC : asCALL_THISCALL);
}

int randomInt(int min, int max) {
    return min + (dist(mt) % (max - min + 1));
}

float randomFloat(float min, float max) {
    return RANGE(min, max);
}

void randomSeed(int32_t seed) {
    srand(seed);
}

void registerMath(asIScriptEngine *engine, bool generic) {
    registerVector2(engine, generic);
    registerVector3(engine, generic);
    registerVector4(engine, generic);

    registerMatrix3(engine, generic);
    registerMatrix4(engine, generic);

    registerQuaternion(engine, generic);

    registerAABBox(engine, generic);
    registerOBBox(engine, generic);

    registerPlane(engine, generic);

    registerRay(engine, generic);

    engine->RegisterGlobalFunction("void seed(int)",
                                   generic ? WRAP_FN(randomSeed) : asFUNCTION(randomSeed),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("int irand(int, int)",
                                   generic ? WRAP_FN(randomInt) : asFUNCTION(randomInt),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);

    engine->RegisterGlobalFunction("float frand(float, float)",
                                   generic ? WRAP_FN(randomFloat) : asFUNCTION(randomFloat),
                                   generic ? asCALL_GENERIC : asCALL_CDECL);
}
