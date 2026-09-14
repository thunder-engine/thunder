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
#ifndef HANDLES_H
#define HANDLES_H

#include <amath.h>

#include <editor.h>

class Mesh;
class Transform;

class EDITOR_EXPORT Handles {
public:
    enum Axises {
        AXIS_X = (1 << 0),
        AXIS_Y = (1 << 1),
        AXIS_Z = (1 << 2)
    };

    enum Sides {
        RIGHT  = (1 << 0),
        LEFT   = (1 << 1),
        TOP    = (1 << 2),
        BOTTOM = (1 << 3),
        FRONT  = (1 << 4),
        BACK   = (1 << 5)
    };

public:
    static void init();

    static void drawArrow(const Matrix4 &transform);
    static void drawBone(const Transform *begin, const Transform *end);

    static Vector3 moveTool(const Vector3 &position, const Quaternion &rotation, bool locked);
    static float rotationTool(const Vector3 &position, const Quaternion &rotation, bool locked);
    static Vector3 scaleTool(const Vector3 &position, const Quaternion &rotation, bool locked);
    static Vector3 rectTool(const Vector3 &center, const Vector3 &box, int &axis, bool side, bool locked);

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

    static float s_Sense;

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
