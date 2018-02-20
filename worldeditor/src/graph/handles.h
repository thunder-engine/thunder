#ifndef HANDLES_H
#define HANDLES_H

#include <vector>

#include <amath.h>

#include <rendersystem.h>
#include <patterns/asingleton.h>

class Texture;
class Camera;
class IRenderSystem;

using namespace std;

class Handles : public ASingleton<Handles> {
public:
    enum Axes {
        AXIS_X  = (1 << 0),
        AXIS_Y  = (1 << 1),
        AXIS_Z  = (1 << 2)
    };

public:
    static void             init                (IRenderSystem *system);

    static void             beginDraw           ();
    static void             endDraw             ();

    static void             drawArrow           (const Matrix4 &transform);

    static void             drawFrustum         (const Vector3Vector &points);

    static void             drawBox             (const Vector3 &position, const Quaternion &rotation, const Vector3 &size);

    static void             drawBillboard       (const Vector3 &position, const Vector2 &size, Texture &texture);

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

    static Vector3          m_sMouse;

    static Matrix4          m_Model;

    static uint8_t          s_Axes;

    static MaterialInstance*s_Instance;

protected:
    static Mesh            *s_Cone;

    static Mesh            *s_Move;

};

#endif // HANDLES_H
