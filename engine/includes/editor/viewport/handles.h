#ifndef HANDLES_H
#define HANDLES_H

#include <amath.h>

#include "resources/mesh.h"

class Texture;
class MaterialInstance;

class Transform;

class CommandBuffer;

class ENGINE_EXPORT Handles {
public:
    enum Axises {
        AXIS_X = (1 << 0),
        AXIS_Y = (1 << 1),
        AXIS_Z = (1 << 2)
    };

    enum Points {
        POINT_T = (1 << 0),
        POINT_B = (1 << 1),
        POINT_L = (1 << 2),
        POINT_R = (1 << 3)
    };

public:
    static void init();

    static void beginDraw(CommandBuffer *buffer);
    static void endDraw();
    static void cleanDepth();

    static void drawArrow(const Matrix4 &transform);
    static void drawBone(const Transform *begin, const Transform *end);
    static void drawDisk(const Vector3 &center, const Quaternion &rotation, float radius, float from, float angle);
    static bool drawBillboard(const Vector3 &position, const Vector2 &size, Texture *texture);

    static void drawLines(const Matrix4 &transform, const Vector3Vector &points, const IndexVector &indices);
    static void drawRectangle(const Vector3 &center, const Quaternion &rotation, float width, float height);
    static void drawCircle(const Vector3 &center, const Quaternion &rotation, float radius);
    static void drawSphere(const Vector3 &center, const Quaternion &rotation, float radius);
    static void drawCapsule(const Vector3 &center, const Quaternion &rotation, float radius, float height);
    static void drawBox(const Vector3 &center, const Quaternion &rotation, const Vector3 &size);

    static Vector3 moveTool(const Vector3 &position, const Quaternion &rotation, bool locked);
    static float rotationTool(const Vector3 &position, const Quaternion &rotation, bool locked);
    static Vector3 scaleTool(const Vector3 &position, const Quaternion &rotation, bool locked);
    static Vector3 rectTool(const Vector3 &position, const Vector3 &box, int &axis, bool locked);

public:
    static Vector4 s_Color;
    static Vector4 s_Second;

    static Vector4 s_Normal;
    static Vector4 s_Grey;
    static Vector4 s_Selected;

    static Vector4 s_xColor;
    static Vector4 s_yColor;
    static Vector4 s_zColor;

    static Vector3 s_World;

    static Vector2 s_Mouse;
    static Vector2 s_Screen;

    static Matrix4 s_Model;

    static uint8_t s_Axes;

    static float s_Sense;

protected:
    static Mesh *s_Cone;
    static Mesh *s_Quad;
    static Mesh *s_Sphere;
    static Mesh *s_Lines;

    static Mesh *s_Axis;
    static Mesh *s_Scale;
    static Mesh *s_ScaleXY;
    static Mesh *s_ScaleXYZ;
    static Mesh *s_Move;
    static Mesh *s_MoveXY;
    static Mesh *s_Arc;
    static Mesh *s_Circle;
    static Mesh *s_Rectangle;
    static Mesh *s_Box;
    static Mesh *s_Bone;

    static MaterialInstance *s_Gizmo;
    static MaterialInstance *s_Solid;
    static MaterialInstance *s_Sprite;

    static Texture *s_Corner;

};

#endif // HANDLES_H
