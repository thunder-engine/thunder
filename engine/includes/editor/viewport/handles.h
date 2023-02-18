#ifndef HANDLES_H
#define HANDLES_H

#include <amath.h>

#include <engine.h>

class Texture;
class Mesh;
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

    static void drawArrow(const Matrix4 &transform);
    static void drawBone(const Transform *begin, const Transform *end);

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

    static uint8_t s_Axes;

protected:
    static Mesh *s_Cone;
    static Mesh *s_Bone;

    static Mesh *s_Axis;
    static Mesh *s_Scale;
    static Mesh *s_ScaleXY;
    static Mesh *s_ScaleXYZ;
    static Mesh *s_Move;
    static Mesh *s_MoveXY;

};

#endif // HANDLES_H
