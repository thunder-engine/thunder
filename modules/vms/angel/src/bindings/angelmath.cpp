#include "bindings/angelbindings.h"

#include <angelscript.h>

#include <amath.h>

static void vec2(Vector2 *dest) {
    new (dest) Vector2();
}

static void deleteVec2(Vector2 *dest) {
    (reinterpret_cast<Vector2 *>(dest))->~Vector2();
}

static void new2Float2(float x, float y, Vector2 *dest) {
    new (dest) Vector2(x, y);
}

static void new1Float2(float value, Vector2 *dest) {
    new (dest) Vector2(value);
}

void registerVector2(asIScriptEngine *engine) {
    engine->RegisterObjectType("Vector2", sizeof(Vector2), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(vec2), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(float)", asFUNCTION(new1Float2), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(float, float)", asFUNCTION(new2Float2), asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector2", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(deleteVec2),  asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector2", "Vector2 &opAssign(const Vector2 &in)", asMETHODPR(Vector2, operator=, (const Vector2&), Vector2&), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "bool opEquals(const Vector2 &in)", asMETHOD(Vector2, operator==), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "bool opCmp(const Vector2 &in)", asMETHOD(Vector2, operator>), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 opMul(float)", asMETHODPR(Vector2, operator*, (areal) const, Vector2), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "Vector2 opMul(const Vector2 &in)", asMETHODPR(Vector2, operator*, (const Vector2 &) const, Vector2), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 opDiv(float)", asMETHOD(Vector2, operator/), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "Vector2 opAdd(const Vector2 &in)", asMETHOD(Vector2, operator+), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "Vector2 opNeg()", asMETHODPR(Vector2, operator-, () const, Vector2), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "Vector2 opSub(const Vector2 &in)", asMETHODPR(Vector2, operator-, (const Vector2 &) const, Vector2), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "Vector2 &opMulAssign(float)", asMETHOD(Vector2, operator*=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "Vector2 &opDivAssign(float)", asMETHOD(Vector2, operator/=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "Vector2 &opAddAssign(const Vector2 &in)", asMETHOD(Vector2, operator+=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "Vector2 &opSubAssign(const Vector2 &in)", asMETHOD(Vector2, operator-=), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "float opIndex(int)", asMETHODPR(Vector2, operator[], (int i) const, float), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector2", "float length()", asMETHOD(Vector2, length), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "float sqrLength()", asMETHOD(Vector2, sqrLength), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "float normalize()", asMETHOD(Vector2, normalize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "Vector2 &cross(const Vector2 &in)", asMETHOD(Vector2, cross), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector2", "float dot(const Vector2 &in)", asMETHOD(Vector2, dot), asCALL_THISCALL);
}

static void vec3(Vector3 *dest) {
    new (dest) Vector3();
}

static void deleteVec3(Vector3 *dest) {
    (reinterpret_cast<Vector3 *>(dest))->~Vector3();
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

void registerVector3(asIScriptEngine *engine) {
    engine->RegisterObjectType("Vector3", sizeof(Vector3), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
    engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(vec3), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(float)", asFUNCTION(new1Float3), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(float, float, float)", asFUNCTION(new3Float3), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(const Vector2 &in, float)", asFUNCTION(newVec2Float3), asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector3", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(deleteVec3),  asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector3", "Vector3 &opAssign(const Vector3 &in)", asMETHODPR(Vector3, operator=, (const Vector3&), Vector3&), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "bool opEquals(const Vector3 &in)", asMETHOD(Vector3, operator==), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "bool opCmp(const Vector3 &in)", asMETHOD(Vector3, operator>), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 opMul(float)", asMETHODPR(Vector3, operator*, (areal) const, Vector3), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "Vector3 opMul(const Vector3 &in)", asMETHODPR(Vector3, operator*, (const Vector3 &) const, Vector3), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 opDiv(float)", asMETHOD(Vector3, operator/), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "Vector3 opAdd(const Vector3 &in)", asMETHOD(Vector3, operator+), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "Vector3 opNeg()", asMETHODPR(Vector3, operator-, () const, Vector3), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "Vector3 opSub(const Vector3 &in)", asMETHODPR(Vector3, operator-, (const Vector3 &) const, Vector3), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "Vector3 &opMulAssign(float)", asMETHOD(Vector3, operator*=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "Vector3 &opDivAssign(float)", asMETHOD(Vector3, operator/=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "Vector3 &opAddAssign(const Vector3 &in)", asMETHOD(Vector3, operator+=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "Vector3 &opSubAssign(const Vector3 &in)", asMETHOD(Vector3, operator-=), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float opIndex(int)", asMETHODPR(Vector3, operator[], (int i) const, float), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float length()", asMETHOD(Vector3, length), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "float sqrLength()", asMETHOD(Vector3, sqrLength), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "float normalize()", asMETHOD(Vector3, normalize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "Vector3 &cross(const Vector3 &in)", asMETHOD(Vector3, cross), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "float dot(const Vector3 &in)", asMETHOD(Vector3, dot), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector3", "float angle(const Vector3 &in)", asMETHOD(Vector3, angle), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector3", "float signedAngle(const Vector3 &in, const Vector3 &in)", asMETHOD(Vector3, signedAngle), asCALL_THISCALL);
}

static void vec4(Vector4 *dest) {
    new (dest) Vector4();
}

static void deleteVec4(Vector4 *dest) {
    (reinterpret_cast<Vector4 *>(dest))->~Vector4();
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

void registerVector4(asIScriptEngine *engine) {
    engine->RegisterObjectType("Vector4", sizeof(Vector4), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(vec4), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f(float)", asFUNCTION(new1Float4), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", asFUNCTION(new4Float4), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f(const Vector2 &in, float, float)", asFUNCTION(newVec2Float4), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, float)", asFUNCTION(newVec3Float4), asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Vector4", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(deleteVec4),  asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Vector4", "Vector4 &opAssign(const Vector4 &in)", asMETHODPR(Vector4, operator=, (const Vector4&), Vector4&), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "bool opEquals(const Vector4 &in)", asMETHOD(Vector4, operator==), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "bool opCmp(const Vector4 &in)", asMETHOD(Vector4, operator>), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 opMul(float)", asMETHODPR(Vector4, operator*, (areal) const, Vector4), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector4", "Vector4 opMul(const Vector4 &in)", asMETHODPR(Vector4, operator*, (const Vector4 &) const, Vector4), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 opDiv(float)", asMETHOD(Vector4, operator/), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector4", "Vector4 opAdd(const Vector4 &in)", asMETHOD(Vector4, operator+), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector4", "Vector4 opNeg()", asMETHODPR(Vector4, operator-, () const, Vector4), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector4", "Vector4 opSub(const Vector4 &in)", asMETHODPR(Vector4, operator-, (const Vector4 &) const, Vector4), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "Vector4 &opMulAssign(float)", asMETHOD(Vector4, operator*=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector4", "Vector4 &opDivAssign(float)", asMETHOD(Vector4, operator/=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector4", "Vector4 &opAddAssign(const Vector4 &in)", asMETHOD(Vector4, operator+=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector4", "Vector4 &opSubAssign(const Vector4 &in)", asMETHOD(Vector4, operator-=), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "float opIndex(int)", asMETHODPR(Vector4, operator[], (int i) const, float), asCALL_THISCALL);

    engine->RegisterObjectMethod("Vector4", "float length()", asMETHOD(Vector4, length), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector4", "float sqrLength()", asMETHOD(Vector4, sqrLength), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector4", "float normalize()", asMETHOD(Vector4, normalize), asCALL_THISCALL);
    engine->RegisterObjectMethod("Vector4", "float dot(const Vector4 &in)", asMETHOD(Vector4, dot), asCALL_THISCALL);
}

static void mat3(Matrix3 *dest) {
    new (dest) Matrix3();
}

static void deleteMat3(Matrix3 *dest) {
    (reinterpret_cast<Matrix3 *>(dest))->~Matrix3();
}

void registerMatrix3(asIScriptEngine *engine) {
    engine->RegisterObjectType("Matrix3", sizeof(Matrix3), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Matrix3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(mat3), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Matrix3", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(deleteMat3),  asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 &opAssign(const Matrix3 &in)", asMETHODPR(Matrix3, operator=, (const Matrix3&), Matrix3&), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "bool opEquals(const Matrix3 &in)", asMETHOD(Matrix3, operator==), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 opMul(float)", asMETHODPR(Matrix3, operator*, (areal) const, Matrix3), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "Vector3 opMul(const Vector3 &in)", asMETHODPR(Matrix3, operator*, (const Vector3 &) const, Vector3), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "Vector4 opMul(const Vector4 &in)", asMETHODPR(Matrix3, operator*, (const Vector4 &) const, Vector4), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "Matrix3 opMul(const Matrix3 &in)", asMETHODPR(Matrix3, operator*, (const Matrix3 &) const, Matrix3), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 opAdd(const Matrix3 &in)", asMETHOD(Matrix3, operator+), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "Matrix3 opSub(const Matrix3 &in)", asMETHOD(Matrix3, operator-), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 &opMulAssign(float)", asMETHODPR(Matrix3, operator*=, (areal), Matrix3&), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "Matrix3 &opMulAssign(const Matrix3 &in)", asMETHODPR(Matrix3, operator*=, (const Matrix3 &), Matrix3&), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 &opAddAssign(const Matrix3 &in)", asMETHOD(Matrix3, operator+=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "Matrix3 &opSubAssign(const Matrix3 &in)", asMETHOD(Matrix3, operator-=), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "float opIndex(int)", asMETHODPR(Matrix3, operator[], (int i) const, float), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "Matrix3 transpose()", asMETHOD(Matrix3, transpose), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "float determinant()", asMETHOD(Matrix3, determinant), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "Matrix3 inverse()", asMETHOD(Matrix3, inverse), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "void zero()", asMETHOD(Matrix3, zero), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "void identity()", asMETHOD(Matrix3, identity), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "void rotate(const Vector3 &in, float angle)", asMETHODPR(Matrix3, rotate, (const Vector3 &, float), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "void rotate(const Vector3 &in)", asMETHODPR(Matrix3, rotate, (const Vector3 &), void), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix3", "void scale(const Vector3 &in)", asMETHOD(Matrix3, scale), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix3", "void orthonormalize()", asMETHOD(Matrix3, orthonormalize), asCALL_THISCALL);
}

static void mat4(Matrix4 *dest) {
    new (dest) Matrix4();
}

static void deleteMat4(Matrix4 *dest) {
    (reinterpret_cast<Matrix4 *>(dest))->~Matrix4();
}

void registerMatrix4(asIScriptEngine *engine) {
    engine->RegisterObjectType("Matrix4", sizeof(Matrix4), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Matrix4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(mat4), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Matrix4", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(deleteMat4),  asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 &opAssign(const Matrix4 &in)", asMETHODPR(Matrix4, operator=, (const Matrix4&), Matrix4&), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "bool opEquals(const Matrix4 &in)", asMETHOD(Matrix4, operator==), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 opMul(float)", asMETHODPR(Matrix4, operator*, (areal) const, Matrix4), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "Vector3 opMul(const Vector3 &in)", asMETHODPR(Matrix4, operator*, (const Vector3 &) const, Vector3), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "Vector4 opMul(const Vector4 &in)", asMETHODPR(Matrix4, operator*, (const Vector4 &) const, Vector4), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "Matrix4 opMul(const Matrix4 &in)", asMETHODPR(Matrix4, operator*, (const Matrix4 &) const, Matrix4), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 opAdd(const Matrix4 &in)", asMETHOD(Matrix4, operator+), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "Matrix4 opSub(const Matrix4 &in)", asMETHOD(Matrix4, operator-), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 &opMulAssign(float)", asMETHODPR(Matrix4, operator*=, (areal), Matrix4&), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "Matrix4 &opMulAssign(const Matrix4 &in)", asMETHODPR(Matrix4, operator*=, (const Matrix4 &), Matrix4&), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix4 &opAddAssign(const Matrix4 &in)", asMETHOD(Matrix4, operator+=), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "Matrix4 &opSubAssign(const Matrix4 &in)", asMETHOD(Matrix4, operator-=), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "float opIndex(int)", asMETHODPR(Matrix4, operator[], (int i) const, float), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Matrix3 rotation()", asMETHOD(Matrix4, rotation), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "Matrix4 transpose()", asMETHOD(Matrix4, transpose), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "float determinant()", asMETHOD(Matrix4, determinant), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "Matrix4 inverse()", asMETHOD(Matrix4, inverse), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void reflect(const Vector4 &in)", asMETHOD(Matrix4, reflect), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "void direction(const Vector3 &in, const Vector3 &in)", asMETHOD(Matrix4, direction), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "Vector3 euler()", asMETHOD(Matrix4, euler), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void zero()", asMETHOD(Matrix4, zero), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "void identity()", asMETHOD(Matrix4, identity), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void rotate(const Vector3 &in, float)", asMETHODPR(Matrix4, rotate, (const Vector3 &, float), void), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "void rotate(const Vector3 &in)", asMETHODPR(Matrix4, rotate, (const Vector3 &), void), asCALL_THISCALL);

    engine->RegisterObjectMethod("Matrix4", "void scale(const Vector3 &in)", asMETHOD(Matrix4, scale), asCALL_THISCALL);
    engine->RegisterObjectMethod("Matrix4", "void translate(const Vector3 &in)", asMETHOD(Matrix4, translate), asCALL_THISCALL);
}

static void quat(Quaternion *dest) {
    new (dest) Quaternion();
}

static void deleteQuat(Quaternion *dest) {
    (reinterpret_cast<Quaternion *>(dest))->~Quaternion();
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

void registerQuaternion(asIScriptEngine *engine) {
    engine->RegisterObjectType("Quaternion", sizeof(Quaternion), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(quat), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, float)", asFUNCTION(quatAxis), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in)", asFUNCTION(quatEuler), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f(const Matrix3 &in)", asFUNCTION(quatMat), asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(deleteQuat),  asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Quaternion", "Quaternion &opAssign(const Quaternion &in)", asMETHODPR(Quaternion, operator=, (const Quaternion&), Quaternion&), asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "bool opEquals(const Quaternion &in)", asMETHOD(Quaternion, operator==), asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "Vector3 opMul(const Vector3 &in)", asMETHODPR(Quaternion, operator*, (const Vector3 &) const, Vector3), asCALL_THISCALL);
    engine->RegisterObjectMethod("Quaternion", "Quaternion opMul(const Vector4 &in)", asMETHODPR(Quaternion, operator*, (const Quaternion &) const, Quaternion), asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "float opIndex(int)", asMETHODPR(Quaternion, operator[], (int i) const, float), asCALL_THISCALL);

    engine->RegisterObjectMethod("Quaternion", "Quaternion inverse()", asMETHOD(Quaternion, inverse), asCALL_THISCALL);
    engine->RegisterObjectMethod("Quaternion", "void mix(const Quaternion &in, const Quaternion &in, float)", asMETHOD(Quaternion, mix), asCALL_THISCALL);
    engine->RegisterObjectMethod("Quaternion", "Matrix3 toMatrix()", asMETHOD(Quaternion, toMatrix), asCALL_THISCALL);
    engine->RegisterObjectMethod("Quaternion", "Vector3 euler()", asMETHOD(Quaternion, euler), asCALL_THISCALL);
}

static void aabb(AABBox *dest) {
    new (dest) AABBox();
}

static void deleteAABB(AABBox *dest) {
    (reinterpret_cast<AABBox *>(dest))->~AABBox();
}

static void aabbSize(const Vector3 &center, const Vector3 &size, AABBox *dest) {
    new (dest) AABBox(center, size);
}

void registerAABBox(asIScriptEngine *engine) {
    engine->RegisterObjectType("AABBox", sizeof(AABBox), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("AABBox", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(aabb), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("AABBox", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, const Vector3 &in)", asFUNCTION(aabbSize), asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("AABBox", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(deleteAABB),  asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("AABBox", "AABBox &opAssign(const AABBox &in)", asMETHODPR(AABBox, operator=, (const AABBox&), AABBox&), asCALL_THISCALL);

    engine->RegisterObjectMethod("AABBox", "AABBox opMul(float)", asMETHODPR(AABBox, operator*, (areal) const, const AABBox), asCALL_THISCALL);
    engine->RegisterObjectMethod("AABBox", "AABBox opMul(const Vector3 &in)", asMETHODPR(AABBox, operator*, (const Vector3 &) const, const AABBox), asCALL_THISCALL);
    engine->RegisterObjectMethod("AABBox", "AABBox opMul(const Matrix4 &in)", asMETHODPR(AABBox, operator*, (const Matrix4 &) const, const AABBox), asCALL_THISCALL);

    //engine->RegisterObjectMethod("AABBox", "bool intersect(const Vector3 &in, float)", asMETHOD(AABBox, intersect), asCALL_THISCALL);
    engine->RegisterObjectMethod("AABBox", "void box(Vector3 &out, Vector3 &out)", asMETHOD(AABBox, box), asCALL_THISCALL);
    engine->RegisterObjectMethod("AABBox", "void setBox(const Vector3 &in, const Vector3 &in)", asMETHODPR(AABBox, setBox, (const Vector3 &, const Vector3 &), void), asCALL_THISCALL);
}

static void obb(OBBox *dest) {
    new (dest) OBBox();
}

static void deleteOBB(OBBox *dest) {
    (reinterpret_cast<OBBox *>(dest))->~OBBox();
}

static void obbSize(const Vector3 &center, const Vector3 &size, OBBox *dest) {
    new (dest) OBBox(center, size);
}

static void obbSizeRot(const Vector3 &center, const Vector3 &size, OBBox *dest) {
    new (dest) OBBox(center, size);
}

void registerOBBox(asIScriptEngine *engine) {
    engine->RegisterObjectType("OBBox", sizeof(OBBox), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("OBBox", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(obb), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("OBBox", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, const Vector3 &in)", asFUNCTION(obbSize), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("OBBox", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, const Vector3 &in, const Quaternion &in)", asFUNCTION(obbSizeRot), asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("OBBox", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(deleteOBB),  asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("OBBox", "OBBox &opAssign(const OBBox &in)", asMETHODPR(OBBox, operator=, (const OBBox&), OBBox&), asCALL_THISCALL);

    engine->RegisterObjectMethod("OBBox", "OBBox opMul(float)", asMETHODPR(OBBox, operator*, (areal) const, const OBBox), asCALL_THISCALL);
    engine->RegisterObjectMethod("OBBox", "OBBox opMul(const Vector3 &in)", asMETHODPR(OBBox, operator*, (const Vector3 &) const, const OBBox), asCALL_THISCALL);

    engine->RegisterObjectMethod("OBBox", "void box(const Vector3 &out, const Vector3 &out)", asMETHOD(OBBox, box), asCALL_THISCALL);
    engine->RegisterObjectMethod("OBBox", "void setBox(const Vector3 &in, const Vector3 &in)", asMETHOD(OBBox, setBox), asCALL_THISCALL);
}

static void plane(Plane *dest) {
    new (dest) Plane();
}

static void planePoints(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, Plane *dest) {
    new (dest) Plane(v1, v2, v3);
}

static void deletePlane(Plane *dest) {
    (reinterpret_cast<Plane *>(dest))->~Plane();
}

void registerPlane(asIScriptEngine *engine) {
    engine->RegisterObjectType("Plane", sizeof(Plane), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Plane", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(plane), asCALL_CDECL_OBJLAST);
    engine->RegisterObjectBehaviour("Plane", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, const Vector3 &in, const Vector3 &in)", asFUNCTION(planePoints), asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Plane", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(deletePlane),  asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Plane", "Plane &opAssign(const Plane &in)", asMETHODPR(Plane, operator=, (const Plane&), Plane&), asCALL_THISCALL);

    engine->RegisterObjectMethod("Plane", "float sqrDistance(const Vector3 &in)", asMETHOD(Plane, sqrDistance), asCALL_THISCALL);
}

static void ray(const Vector3 &position, const Vector3 &direction, Ray *dest) {
    new (dest) Ray(position, direction);
}

static void deleteRay(Ray *dest) {
    (reinterpret_cast<Ray *>(dest))->~Ray();
}

void registerRay(asIScriptEngine *engine) {
    engine->RegisterObjectType("Ray", sizeof(Ray), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);

    engine->RegisterObjectBehaviour("Ray", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in, const Vector3 &in)", asFUNCTION(ray), asCALL_CDECL_OBJLAST);

    engine->RegisterObjectBehaviour("Ray", asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(deleteRay),  asCALL_CDECL_OBJLAST);

    engine->RegisterObjectMethod("Ray", "OBBox &opAssign(const OBBox &in)", asMETHODPR(Ray, operator=, (const Ray&), Ray&), asCALL_THISCALL);

    engine->RegisterObjectMethod("Ray", "bool intersect(const Vector3 &in, float, Vector3 &out)", asMETHODPR(Ray, intersect, (const Vector3&, areal, Vector3 *), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("Ray", "bool intersect(const Plane &in, Vector3 &out, bool)", asMETHODPR(Ray, intersect, (const Plane &, Vector3 *, bool), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("Ray", "bool intersect(const AABBox &in, Vector3 &out)", asMETHODPR(Ray, intersect, (const AABBox&, Vector3 *), bool), asCALL_THISCALL);
    engine->RegisterObjectMethod("Ray", "bool intersect(const Vector3 &in, const Vector3 &in, const Vector3 &in, Vector3 &out, bool)", asMETHODPR(Ray, intersect, (const Vector3 &, const Vector3 &, const Vector3 &, Vector3 *, bool), bool), asCALL_THISCALL);

    engine->RegisterObjectMethod("Ray", "Ray reflect(const Vector3 &in, const Vector3 &in)", asMETHOD(Ray, reflect), asCALL_THISCALL);
    engine->RegisterObjectMethod("Ray", "Ray refract(const Vector3 &in, const Vector3 &in, float)", asMETHOD(Ray, refract), asCALL_THISCALL);
    engine->RegisterObjectMethod("Ray", "Ray diffuse(const Vector3 &in, const Vector3 &in, float, float)", asMETHOD(Ray, diffuse), asCALL_THISCALL);
}

int randomInt(int min, int max) {
    return min + (dist(mt) % (max - min + 1));
}

float randomFloat(float min, float max) {
    return RANGE(min, max);
}

void registerMath(asIScriptEngine *engine) {
    registerVector2(engine);
    registerVector3(engine);
    registerVector4(engine);

    registerMatrix3(engine);
    registerMatrix4(engine);

    registerQuaternion(engine);

    registerAABBox(engine);
    registerOBBox(engine);

    registerPlane(engine);

    registerRay(engine);

    engine->RegisterGlobalFunction("void seed(int)", asFUNCTION(srand), asCALL_CDECL);
    engine->RegisterGlobalFunction("int irand(int, int)", asFUNCTION(randomInt), asCALL_CDECL);
    engine->RegisterGlobalFunction("float frand(float, float)", asFUNCTION(randomFloat), asCALL_CDECL);
}
