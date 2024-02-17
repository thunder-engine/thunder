#ifndef GIZMOS_H
#define GIZMOS_H

#include <amath.h>

#include <engine.h>

class Mesh;
class Sprite;
class Material;
class MaterialInstance;

class CommandBuffer;

class ENGINE_EXPORT Gizmos {
public:
    static void drawBox(const Vector3 &center, const Vector3 &size, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawIcon(const Vector3 &center, const Vector2 &size, const string &name, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawMesh(Mesh &mesh, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawSphere(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawSolidArc(const Vector3 &center, float radius, float start, float angle, const Vector4 &color, const Matrix4 &transform = Matrix4());

    static void drawLines(const Vector3Vector &points, const IndexVector &indices, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawArc(const Vector3 &center, float radius, float start, float angle, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawCircle(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawRectangle(const Vector3 &center, const Vector2 &size, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawWireBox(const Vector3 &center, const Vector3 &size, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawWireMesh(Mesh &mesh, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawWireSphere(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 &transform = Matrix4());
    static void drawWireCapsule(const Vector3 &center, float radius, float height, const Vector4 &color, const Matrix4 &transform = Matrix4());

private:
    friend class GizmoRender;

    static void init();

    static void clear();
    static void drawSpriteBatch(CommandBuffer *buffer);
    static void drawSolidBatch(CommandBuffer *buffer);
    static void drawWireBatch(CommandBuffer *buffer);

protected:
    static Matrix4 s_view;
    static Matrix4 s_projection;

    static Mesh *s_wire;
    static Mesh *s_solid;

    static Material *s_spriteMaterial;

    static MaterialInstance *s_wireMaterial;
    static MaterialInstance *s_solidMaterial;

};

#endif // GIZMOS_H
