#ifndef HANDLES_H
#define HANDLES_H

#include <vector>

#include <amath.h>

#include <resources/mesh.h>

class Texture;
class Camera;

class ICommandBuffer;

using namespace std;

class Handles {
public:
    enum Axes {
        AXIS_X  = (1 << 0),
        AXIS_Y  = (1 << 1),
        AXIS_Z  = (1 << 2)
    };

public:
    static void             init                ();

    static void             beginDraw           (ICommandBuffer *buffer);
    static void             endDraw             ();

    static void             drawArrow           (const Matrix4 &transform);

    static void             drawLines           (const Matrix4 &transform, const Vector3Vector &points, const Mesh::IndexVector &indices);

    static void             drawAABB            (AABBox &box);

    static bool             drawBillboard       (const Vector3 &position, const Vector2 &size, Texture *texture);

    static Vector3          moveTool            (const Vector3 &position, bool locked);

    static Vector3          rotationTool        (const Vector3 &position, bool locked);

    static Vector3          scaleTool           (const Vector3 &position, bool locked);

public:
    static Vector4          s_Color;
    static Vector4          s_Second;

    static Vector4          s_Normal;
    static Vector4          s_Selected;

    static Vector4          s_xColor;
    static Vector4          s_yColor;
    static Vector4          s_zColor;

    static Camera          *s_ActiveCamera;

    static Vector2          m_sMouse;
    static Vector2          m_sScreen;

    static Matrix4          m_Model;

    static uint8_t          s_Axes;

protected:
    static Mesh            *s_Cone;

    static Mesh            *s_Quad;

    static Mesh            *s_Move;

    static Mesh            *s_Lines;

    static MaterialInstance*s_Gizmo;

    static MaterialInstance*s_Sprite;

};

#endif // HANDLES_H
