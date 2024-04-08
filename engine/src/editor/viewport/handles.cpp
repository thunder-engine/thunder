#include "editor/viewport/handles.h"
#include "editor/viewport/handletools.h"

#include "components/camera.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"
#include "resources/mesh.h"

#include "gizmos.h"

#define ALPHA 0.3f

#define CONTROL_SIZE 90.0f

float s_Sense = 0.04f;

Vector4 Handles::s_Normal = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
Vector4 Handles::s_Grey = Vector4(0.3f, 0.3f, 0.3f, 0.6f);
Vector4 Handles::s_Selected = Vector4(1.0f, 1.0f, 0.0f, 1.0f);
Vector4 Handles::s_xColor = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
Vector4 Handles::s_yColor = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
Vector4 Handles::s_zColor = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

Vector4 Handles::s_Color = Handles::s_Normal;
Vector4 Handles::s_Second = Handles::s_Normal;

Vector3 Handles::s_World = Vector3();

Vector2 Handles::s_Mouse = Vector2();
Vector2 Handles::s_Screen = Vector2();

uint8_t Handles::s_Axes = 0;

static float length = 1.0f;
static float conesize = length / 5.0f;

static float s_AngleBegin = 0.0f;
static float s_AngleTotal = 0.0f;

Mesh *Handles::s_Cone = nullptr;
Mesh *Handles::s_Bone = nullptr;

Mesh *Handles::s_Axis = nullptr;
Mesh *Handles::s_Scale = nullptr;
Mesh *Handles::s_ScaleXY = nullptr;
Mesh *Handles::s_ScaleXYZ = nullptr;
Mesh *Handles::s_Move = nullptr;
Mesh *Handles::s_MoveXY = nullptr;

enum {
    AXIS,
    SCALE,
    SCALE_XY,
    SCALE_XYZ,
    MOVE,
    MOVE_XY,
    CIRCLE
};

void Handles::init() {
    if(s_Cone == nullptr) {
        s_Cone = Engine::loadResource<Mesh>(".embedded/cone.fbx/Cone001");
    }
    if(s_Bone == nullptr) {
        s_Bone = Engine::loadResource<Mesh>(".embedded/bone.fbx/Bone");
    }

    if(s_Axis == nullptr) {
        s_Axis = Engine::objectCreate<Mesh>("Axis");
        s_Axis->setVertices({Vector3(0.0f), Vector3(0, 5, 0)});
        s_Axis->setIndices({0, 1});
    }

    if(s_Scale == nullptr) {
        s_Scale = Engine::objectCreate<Mesh>("Scale");
        s_Scale->setVertices({Vector3(0, 2, 0), Vector3(1, 1, 0), Vector3(0, 3, 0), Vector3(1.5, 1.5, 0)});
        s_Scale->setIndices({0, 1, 2, 3});
    }

    if(s_ScaleXY == nullptr) {
        s_ScaleXY = Engine::objectCreate<Mesh>("ScaleXY");
        s_ScaleXY->setVertices({Vector3(0, 2, 0), Vector3(1, 1, 0), Vector3(0, 3, 0), Vector3(1.5, 1.5, 0)});
        s_ScaleXY->setIndices({0, 1, 2, 1, 3, 2});
    }

    if(s_ScaleXYZ == nullptr) {
        s_ScaleXYZ = Engine::objectCreate<Mesh>("ScaleXYZ");
        s_ScaleXYZ->setVertices({Vector3(0, 2, 0), Vector3(0, 0, 0), Vector3(1, 1, 0)});
        s_ScaleXYZ->setIndices({0, 1, 2});
    }

    if(s_Move == nullptr) {
        s_Move = Engine::objectCreate<Mesh>("Move");
        s_Move->setVertices({Vector3(0, 1, 0), Vector3(2, 1, 0)});
        s_Move->setIndices({0, 1});
    }

    if(s_MoveXY == nullptr) {
        s_MoveXY = Engine::objectCreate<Mesh>("MoveXY");
        s_MoveXY->setVertices({Vector3(0,-1, 0), Vector3(2,-1, 0), Vector3(0, 1, 0), Vector3(2, 1, 0)});
        s_MoveXY->setIndices({0, 1, 2, 1, 3, 2});
    }
}

void Handles::drawArrow(const Matrix4 &transform) {
    Gizmos::drawLines({Vector3(), Vector3(0, 4, 0)}, {0, 1}, s_Color, transform);

    Matrix4 t;
    t[13] += 4.0f;

    Gizmos::drawMesh(*s_Cone, s_Second, transform * t);
}

void Handles::drawBone(const Transform *begin, const Transform *end) {
    Vector3 p0 = begin->worldPosition();
    Vector3 p1 = end->worldPosition();

    float size = (p1 - p0).length() * 0.1f;
    Matrix4 b(p0, begin->worldQuaternion(), Vector3(size));

    Gizmos::drawMesh(*s_Bone, s_Color, b);
}

Vector3 Handles::moveTool(const Vector3 &position, const Quaternion &rotation, bool locked) {
    Vector3 result = position;
    Camera *camera = Camera::current();
    if(camera) {
        Vector3 normal = position - camera->transform()->position();
        float scale = normal.normalize();
        if(camera->orthographic()) {
            scale = camera->orthoSize();
        }
        scale *= (CONTROL_SIZE / s_Screen.y);
        Matrix4 model(position, rotation, scale);

        Matrix4 x = model * Matrix4(Vector3(conesize, 0, 0), Quaternion(Vector3(0, 0, 1),-90) * Quaternion(Vector3(0, 1, 0),-90), conesize);
        Matrix4 y = model * Matrix4(Vector3(0, conesize, 0), Quaternion(), conesize);
        Matrix4 z = model * Matrix4(Vector3(0, 0, conesize), Quaternion(Vector3(0, 0, 1), 90) * Quaternion(Vector3(1, 0, 0), 90), conesize);

        Matrix4 r(Vector3(), Quaternion(Vector3(0, 1, 0),-90), Vector3(1));

        if(!locked) {
            if(HandleTools::distanceToPoint(model, Vector3(), s_Mouse) <= s_Sense) {
                s_Axes = AXIS_X | AXIS_Y | AXIS_Z;
            } else if((HandleTools::distanceToMesh(x, s_Move->indices(), s_Move->vertices(), s_Mouse) <= s_Sense) ||
                      (HandleTools::distanceToMesh(z * r, s_Move->indices(), s_Move->vertices(), s_Mouse) <= s_Sense)) {
                s_Axes = AXIS_X | AXIS_Z;
            } else if((HandleTools::distanceToMesh(y, s_Move->indices(), s_Move->vertices(), s_Mouse) <= s_Sense) ||
                      (HandleTools::distanceToMesh(x * r, s_Move->indices(), s_Move->vertices(), s_Mouse) <= s_Sense)) {
                s_Axes = AXIS_Y | AXIS_X;
            } else if((HandleTools::distanceToMesh(z, s_Move->indices(), s_Move->vertices(), s_Mouse) <= s_Sense) ||
                      (HandleTools::distanceToMesh(y * r, s_Move->indices(), s_Move->vertices(), s_Mouse) <= s_Sense)) {
                s_Axes = AXIS_Z | AXIS_Y;
            } else if(HandleTools::distanceToMesh(x, s_Axis->indices(), s_Axis->vertices(), s_Mouse) <= s_Sense) {
                s_Axes = AXIS_X;
            } else if(HandleTools::distanceToMesh(y, s_Axis->indices(), s_Axis->vertices(), s_Mouse) <= s_Sense) {
                s_Axes = AXIS_Y;
            } else if(HandleTools::distanceToMesh(z, s_Axis->indices(), s_Axis->vertices(), s_Mouse) <= s_Sense) {
                s_Axes = AXIS_Z;
            }
        }

        s_Second = s_xColor;
        s_Color = (s_Axes & AXIS_X) ? s_Selected : s_xColor;
        drawArrow(x);
        Gizmos::drawWireMesh(*s_Move, s_Axes == (AXIS_X | AXIS_Z) ? s_Selected : s_xColor, x);
        Gizmos::drawWireMesh(*s_Move, s_Axes == (AXIS_X | AXIS_Y) ? s_Selected : s_xColor, x * r);

        s_Second = s_yColor;
        s_Color = (s_Axes & AXIS_Y) ? s_Selected : s_yColor;
        drawArrow(y);
        Gizmos::drawWireMesh(*s_Move, s_Axes == (AXIS_X | AXIS_Y) ? s_Selected : s_yColor, y);
        Gizmos::drawWireMesh(*s_Move, s_Axes == (AXIS_Y | AXIS_Z) ? s_Selected : s_yColor, y * r);

        s_Second = s_zColor;
        s_Color = (s_Axes & AXIS_Z) ? s_Selected : s_zColor;
        drawArrow(z);
        Gizmos::drawWireMesh(*s_Move, s_Axes == (AXIS_Y | AXIS_Z) ? s_Selected : s_zColor, z);
        Gizmos::drawWireMesh(*s_Move, s_Axes == (AXIS_X | AXIS_Z) ? s_Selected : s_zColor, z * r);

        Vector4 selected(s_Selected);
        selected.w = ALPHA;
        if(s_Axes == (AXIS_X | AXIS_Z)) {
            Gizmos::drawMesh(*s_MoveXY, selected, x);
        }
        if(s_Axes == (AXIS_X | AXIS_Y)) {
            Gizmos::drawMesh(*s_MoveXY, selected, y);
        }
        if(s_Axes == (AXIS_Y | AXIS_Z)) {
            Gizmos::drawMesh(*s_MoveXY, selected, z);
        }

        Plane plane;
        plane.point = position;
        plane.normal = camera->transform()->quaternion() * Vector3(0.0f, 0.0f, 1.0f);
        if(s_Axes == AXIS_X) {
            plane.normal = rotation * Vector3(0.0f, plane.normal.y, plane.normal.z);
        } else if(s_Axes == AXIS_Z) {
            plane.normal = rotation * Vector3(plane.normal.x, plane.normal.y, 0.0f);
        } else if(s_Axes == (AXIS_X | AXIS_Z)) {
            plane.normal = rotation * Vector3(0.0f, 1.0f, 0.0f);
        } else if(s_Axes == (AXIS_X | AXIS_Y)) {
            plane.normal = rotation * Vector3(0.0f, 0.0f, 1.0f);
        } else if(s_Axes == (AXIS_Z | AXIS_Y)) {
            plane.normal = rotation * Vector3(1.0f, 0.0f, 0.0f);
        } else if(s_Axes == AXIS_Y || s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) {
            plane.normal = rotation * Vector3(plane.normal.x, 0.0f, plane.normal.z);
        }
        plane.normal.normalize();
        plane.d = plane.normal.dot(plane.point);

        Ray ray = camera->castRay(s_Mouse.x, s_Mouse.y);
        Ray::Hit hit;
        ray.intersect(plane, &hit, true);

        Vector3 mask;
        if(s_Axes & AXIS_X) {
            mask += Vector3(1, 0, 0);
        }
        if(s_Axes & AXIS_Y) {
            mask += Vector3(0, 1, 0);
        }
        if(s_Axes & AXIS_Z) {
            mask += Vector3(0, 0, 1);
        }

        result = rotation.inverse() * hit.point * mask;
    }
    return result;
}

float Handles::rotationTool(const Vector3 &position, const Quaternion &rotation, bool locked) {
    Camera *camera = Camera::current();
    if(camera) {
        Transform *t = camera->transform();
        Vector3 cameraDelta = position - t->position();
        float scale = 1.0f;
        if(!camera->orthographic()) {
            scale = cameraDelta.length();
        } else {
            scale = camera->orthoSize();
        }
        scale *= (CONTROL_SIZE / s_Screen.y);

        cameraDelta = rotation.inverse() * cameraDelta;
        cameraDelta.normalize();

        Matrix4 model(position, rotation, scale * 5.0f);

        Matrix4 q1 = Matrix4(position, Quaternion(), scale * 5.0f) *
                     Matrix4(Vector3(), t->quaternion() * Quaternion(Vector3(90, 0, 0)), Vector3(conesize));

        Matrix4 x = model * Matrix4(Vector3(), Quaternion(Vector3( 0, 0, 90)) *
                                               Quaternion(Vector3( 0, 1, 0), RAD2DEG * atan2(cameraDelta.y, cameraDelta.z) + 180), Vector3(conesize));

        Matrix4 y = model * Matrix4(Vector3(), Quaternion(Vector3( 0, 1, 0), RAD2DEG * atan2(cameraDelta.x, cameraDelta.z) + 180), Vector3(conesize));

        Matrix4 z = model * Matrix4(Vector3(), Quaternion(Vector3( 0, 0, 1),-RAD2DEG * atan2(cameraDelta.x, cameraDelta.y)) *
                                               Quaternion(Vector3(90, 0, 0)), Vector3(conesize));
        Matrix4 m;
        m.scale(1.2f);

        if(!locked) {
            Vector3Vector c_points = Mathf::pointsArc(Quaternion(), 1.0f, 0, 360, 64);
            IndexVector c_indices;
            c_indices.resize((c_points.size() - 1) * 2);
            for(int i = 0; i < c_points.size() - 1; i++) {
                c_indices[i * 2] = i;
                c_indices[i * 2 + 1] = i+1;
            }

            Vector3Vector a_points = Mathf::pointsArc(Quaternion(), 1.0f, 0, 180, 64);
            IndexVector a_indices;
            a_indices.resize((a_points.size() - 1) * 2);
            for(int i = 0; i < a_points.size() - 1; i++) {
                a_indices[i * 2] = i;
                a_indices[i * 2 + 1] = i+1;
            }

            float xyzDist = HandleTools::distanceToMesh(q1 * m, c_indices, c_points, s_Mouse);
            float xDist = HandleTools::distanceToMesh(x, a_indices, a_points, s_Mouse);
            float yDist = HandleTools::distanceToMesh(y, a_indices, a_points, s_Mouse);
            float zDist = HandleTools::distanceToMesh(z, a_indices, a_points, s_Mouse);

            if(xyzDist <= s_Sense) {
                s_Axes = AXIS_X | AXIS_Y | AXIS_Z;
            }
            if(xDist <= s_Sense && (xDist < yDist) && (xDist < zDist)) {
                s_Axes = AXIS_X;
            }
            if(yDist <= s_Sense && (yDist < xDist) && (yDist < zDist)) {
                s_Axes = AXIS_Y;
            }
            if(zDist <= s_Sense && (zDist < xDist) && (zDist < yDist)) {
                s_Axes = AXIS_Z;
            }
        }

        Plane plane;
        plane.point = position;
        plane.normal = t->quaternion() * Vector3(0.0f, 0.0f, 1.0f);
        if(s_Axes == AXIS_X) {
            plane.normal = Vector3(1.0f, 0.0f, 0.0f);
        } else if(s_Axes == AXIS_Y) {
            plane.normal = Vector3(0.0f, 1.0f, 0.0f);
        } else if(s_Axes == AXIS_Z) {
            plane.normal = Vector3(0.0f, 0.0f, 1.0f);
        }
        plane.d = plane.normal.dot(plane.point);

        Ray ray = camera->castRay(s_Mouse.x, s_Mouse.y);
        Ray::Hit hit;
        ray.intersect(plane, &hit, true);

        Vector3 dt0 = hit.point - position;
        if(locked) {
            Vector3 dt1 = s_World - position;
            float angle = dt1.signedAngle(dt0, plane.normal);

            s_AngleTotal += angle;
            Vector4 selected = s_Selected;
            selected.w = ALPHA;
            if(s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) {
                Matrix4 t = q1 * m;
                Gizmos::drawSolidArc(Vector3(), 1.0f, s_AngleBegin, -s_AngleTotal, selected, t);
                Gizmos::drawCircle(Vector3(), 1.0f, selected, t);
            } else if(s_Axes == AXIS_X) {
                Gizmos::drawSolidArc(Vector3(), 1.0f, s_AngleBegin + 45, s_AngleTotal, selected, x);
                Gizmos::drawCircle(Vector3(), 1.0f, selected, x);
            } else if(s_Axes == AXIS_Y) {
                Gizmos::drawSolidArc(Vector3(), 1.0f, s_AngleBegin + 45, -s_AngleTotal, selected, y);
                Gizmos::drawCircle(Vector3(), 1.0f, selected, y);
            } else if(s_Axes == AXIS_Z) {
                Gizmos::drawSolidArc(Vector3(), 1.0f, s_AngleBegin + 135, -s_AngleTotal, selected, z);
                Gizmos::drawCircle(Vector3(), 1.0f, selected, z);
            }
        } else {
            if(camera->orthographic()) {
                Gizmos::drawCircle(Vector3(), 1.0f, s_Color = (s_Axes == AXIS_X) ? s_Selected : s_xColor, x);
                Gizmos::drawCircle(Vector3(), 1.0f, s_Color = (s_Axes == AXIS_Y) ? s_Selected : s_yColor, y);
                Gizmos::drawCircle(Vector3(), 1.0f, s_Color = (s_Axes == AXIS_Z) ? s_Selected : s_zColor, z);
            } else {
                Gizmos::drawArc(Vector3(), 1.0f, 0.0f, 180.0f, s_Color = (s_Axes == AXIS_X) ? s_Selected : s_xColor, x);
                Gizmos::drawArc(Vector3(), 1.0f, 0.0f, 180.0f, s_Color = (s_Axes == AXIS_Y) ? s_Selected : s_yColor, y);
                Gizmos::drawArc(Vector3(), 1.0f, 0.0f, 180.0f, s_Color = (s_Axes == AXIS_Z) ? s_Selected : s_zColor, z);
            }

            if(s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) {
                s_AngleBegin = dt0.signedAngle(t->quaternion() * Vector3(1.0f, 0.0f, 0.0f), plane.normal);
            } else if(s_Axes == AXIS_X) {
                s_AngleBegin =-dt0.signedAngle(Vector3(0.0f, 1.0f, 0.0f), plane.normal);
            } else {
                s_AngleBegin = dt0.signedAngle(Vector3(1.0f, 0.0f, 0.0f), plane.normal);
            }
            s_AngleTotal = 0.0f;
        }
        s_Color = (s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) ? s_Selected : s_Grey * 2.0f;

        Matrix4 mat = q1 * m;
        Gizmos::drawCircle(Vector3(), 1.0f, s_Color, mat);
        Gizmos::drawCircle(Vector3(), 1.0f, s_Grey, q1);

        s_Color = s_Normal;
        s_World = hit.point;
    }

    return s_AngleTotal;
}

Vector3 Handles::scaleTool(const Vector3 &position, const Quaternion &rotation, bool locked) {
    Vector3 result;
    Camera *camera = Camera::current();
    if(camera) {
        Vector3 normal = position - camera->transform()->position();
        float size = 1.0f;
        if(!camera->orthographic()) {
            size = normal.length();
        } else {
            size = camera->orthoSize();
        }
        size *= (CONTROL_SIZE / s_Screen.y);
        Vector3 scale(((normal.x < 0) ? 1 : -1) * size,
                      ((normal.y < 0) ? 1 : -1) * size,
                      ((normal.z < 0) ? 1 : -1) * size);

        Matrix4 model(position, rotation, scale);
        Matrix4 x = model * Matrix4(Vector3(), Quaternion(Vector3(0, 0, 1),-90) * Quaternion(Vector3(0, 1, 0),-90), Vector3(conesize));
        Matrix4 y = model * Matrix4(Vector3(), Quaternion(), Vector3(conesize));
        Matrix4 z = model * Matrix4(Vector3(), Quaternion(Vector3(0, 0, 1), 90) * Quaternion(Vector3(1, 0, 0), 90), Vector3(conesize));

        Matrix4 r(Vector3(), Quaternion(Vector3(0, 1, 0),-90), Vector3(1));

        if(!locked) {
            if(HandleTools::distanceToPoint(model, Vector3(), s_Mouse) <= s_Sense) {
                s_Axes = AXIS_X | AXIS_Y | AXIS_Z;
            } else if((HandleTools::distanceToMesh(x, s_Scale->indices(), s_Scale->vertices(), s_Mouse) <= s_Sense) ||
                      (HandleTools::distanceToMesh(z * r, s_Scale->indices(), s_Scale->vertices(), s_Mouse) <= s_Sense)) {
                s_Axes = AXIS_X | AXIS_Z;
            } else if((HandleTools::distanceToMesh(y, s_Scale->indices(), s_Scale->vertices(), s_Mouse) <= s_Sense) ||
                      (HandleTools::distanceToMesh(x * r, s_Scale->indices(), s_Scale->vertices(), s_Mouse) <= s_Sense)) {
                s_Axes = AXIS_Y | AXIS_X;
            } else if((HandleTools::distanceToMesh(z, s_Scale->indices(), s_Scale->vertices(), s_Mouse) <= s_Sense) ||
                      (HandleTools::distanceToMesh(y * r, s_Scale->indices(), s_Scale->vertices(), s_Mouse) <= s_Sense)) {
                s_Axes = AXIS_Z | AXIS_Y;
            } else if(HandleTools::distanceToMesh(x, s_Axis->indices(), s_Axis->vertices(), s_Mouse) <= s_Sense) {
                s_Axes = AXIS_X;
            } else if(HandleTools::distanceToMesh(y, s_Axis->indices(), s_Axis->vertices(), s_Mouse) <= s_Sense) {
                s_Axes = AXIS_Y;
            } else if(HandleTools::distanceToMesh(z, s_Axis->indices(), s_Axis->vertices(), s_Mouse) <= s_Sense) {
                s_Axes = AXIS_Z;
            }
        }

        s_Color = s_Selected;
        s_Color.w = ALPHA;
        Matrix4 xr = x * r;
        Matrix4 yr = y * r;
        Matrix4 zr = z * r;
        {
            Vector4 color(s_xColor);
            if(s_Axes == (AXIS_X | AXIS_Z)) {
                Gizmos::drawMesh(*s_ScaleXY, s_Color, x);
                color = s_Selected;
            }
            Gizmos::drawWireMesh(*s_Scale, color, x);

            color = s_xColor;
            if(s_Axes == (AXIS_X | AXIS_Y)) {
                Gizmos::drawMesh(*s_ScaleXY, s_Color, xr);
                color = s_Selected;
            }
            Gizmos::drawWireMesh(*s_Scale, color, xr);
            Gizmos::drawWireMesh(*s_Axis, (s_Axes & AXIS_X) ? s_Selected : s_xColor, x);
        }
        {
            Vector4 color(s_yColor);
            if(s_Axes == (AXIS_Y | AXIS_X)) {
                Gizmos::drawMesh(*s_ScaleXY, s_Color, y);
                color = s_Selected;
            }
            Gizmos::drawWireMesh(*s_Scale, color, y);

            color = s_yColor;
            if(s_Axes == (AXIS_Y | AXIS_Z)) {
                Gizmos::drawMesh(*s_ScaleXY, s_Color, yr);
                color = s_Selected;
            }
            Gizmos::drawWireMesh(*s_Scale, color, yr);
            Gizmos::drawWireMesh(*s_Axis, (s_Axes & AXIS_Y) ? s_Selected : s_yColor, y);
        }
        {
            Vector4 color(s_zColor);
            if(s_Axes == (AXIS_Z | AXIS_Y)) {
                Gizmos::drawMesh(*s_ScaleXY, s_Color, z);
                color = s_Selected;
            }
            Gizmos::drawWireMesh(*s_Scale, color, z);

            color = s_zColor;
            if(s_Axes == (AXIS_Z | AXIS_X)) {
                Gizmos::drawMesh(*s_ScaleXY, s_Color, zr);
                color = s_Selected;
            }
            Gizmos::drawWireMesh(*s_Scale, color, zr);
            Gizmos::drawWireMesh(*s_Axis, (s_Axes & AXIS_Z) ? s_Selected : s_zColor, z);
        }

        if(s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) {
            s_ScaleXYZ->setColors(Vector4Vector(s_ScaleXYZ->vertices().size(), s_Color));
            Gizmos::drawMesh(*s_ScaleXYZ, s_Color, x);
            Gizmos::drawMesh(*s_ScaleXYZ, s_Color, xr);
            Gizmos::drawMesh(*s_ScaleXYZ, s_Color, y);
            Gizmos::drawMesh(*s_ScaleXYZ, s_Color, yr);
            Gizmos::drawMesh(*s_ScaleXYZ, s_Color, z);
            Gizmos::drawMesh(*s_ScaleXYZ, s_Color, zr);
        }
        s_Color = s_Normal;

        Plane plane;
        plane.point = position;
        plane.normal = camera->transform()->quaternion() * Vector3(0.0f, 0.0f, 1.0f);
        if(s_Axes == AXIS_X) {
            plane.normal = Vector3(0.0f, plane.normal.y, plane.normal.z);
        } else if(s_Axes == AXIS_Z) {
            plane.normal = Vector3(plane.normal.x, plane.normal.y, 0.0f);
        } else if(s_Axes == (AXIS_X | AXIS_Z)) {
            plane.normal = Vector3(0.0f, 1.0f, 0.0f);
        } else if(s_Axes == (AXIS_X | AXIS_Y)) {
            plane.normal = Vector3(0.0f, 0.0f, 1.0f);
        } else if(s_Axes == (AXIS_Z | AXIS_Y)) {
            plane.normal = Vector3(1.0f, 0.0f, 0.0f);
        } else if(s_Axes == AXIS_Y || s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) {
            plane.normal = Vector3(plane.normal.x, 0.0f, plane.normal.z);
        }
        plane.normal.normalize();
        plane.d = plane.normal.dot(plane.point);

        Ray ray = camera->castRay(s_Mouse.x, s_Mouse.y);
        Ray::Hit hit;
        ray.intersect(plane, &hit, true);
        if(s_Axes & AXIS_X) {
            result.x = hit.point.x;
        }
        if(s_Axes & AXIS_Y) {
            result.y = hit.point.y;
        }
        if(s_Axes & AXIS_Z) {
            result.z = hit.point.z;
        }
    }
    return result;
}

Vector3 Handles::rectTool(const Vector3 &position, const Vector3 &box, int &axis, bool locked) {
    Vector3 result;
    Camera *camera = Camera::current();
    if(camera) {
        axis = Handles::AXIS_Z;

        Plane plane;
        plane.normal = Vector3(0.0f, 0.0f, 1.0f);

        Vector2 size(box.x, box.y);

        Quaternion q;

        Vector3 normal = camera->transform()->quaternion() * plane.normal;
        normal.normalize();
        if(abs(normal.x) > abs(normal.z)) {
            axis = Handles::AXIS_X;
            plane.normal = Vector3(1.0f, 0.0f, 0.0f);
            size = Vector2(box.z, box.y);
            q = Quaternion(Vector3(0.0f, 90.0f, 0.0f));
        }
        if(abs(normal.y) > abs(normal.x)) {
            axis = Handles::AXIS_Y;
            plane.normal = Vector3(0.0f, 1.0f, 0.0f);
            size = Vector2(box.x, box.z);
            q = Quaternion(Vector3(90.0f, 0.0f, 0.0f));
        }

        plane.point = position;
        plane.d = plane.normal.dot(plane.point);

        Matrix4 model(position, q, Vector3(1.0f));

        Vector3 tr(size.x * 0.5f, size.y * 0.5f, 0.0f);
        Vector3 tl(size.x *-0.5f, size.y * 0.5f, 0.0f);
        Vector3 br(size.x * 0.5f, size.y *-0.5f, 0.0f);
        Vector3 bl(size.x *-0.5f, size.y *-0.5f, 0.0f);

        Gizmos::drawRectangle(Vector3(), Vector2(size.x, size.y), s_Normal, model);

        Transform *t = camera->transform();
        normal = position - t->position();
        float scale = 1.0f;
        if(!camera->orthographic()) {
            scale = normal.length();
        } else {
            scale = camera->orthoSize();
        }
        scale *= (CONTROL_SIZE / s_Screen.y);

        Gizmos::drawBox(model * tr, Vector3(scale * 0.05f), s_zColor);
        Gizmos::drawBox(model * tl, Vector3(scale * 0.05f), s_zColor);
        Gizmos::drawBox(model * br, Vector3(scale * 0.05f), s_zColor);
        Gizmos::drawBox(model * bl, Vector3(scale * 0.05f), s_zColor);

        if(!locked) {
            float sence = s_Sense * 0.25f;

            Handles::s_Axes = 0;
            if(HandleTools::distanceToPoint(model, tr, s_Mouse) <= sence) {
                Handles::s_Axes = Handles::POINT_T | Handles::POINT_R;
            } else if(HandleTools::distanceToPoint(model, tl, s_Mouse) <= sence) {
                Handles::s_Axes = Handles::POINT_T | Handles::POINT_L;
            } else if(HandleTools::distanceToPoint(model, br, s_Mouse) <= sence) {
                Handles::s_Axes = Handles::POINT_B | Handles::POINT_R;
            } else if(HandleTools::distanceToPoint(model, bl, s_Mouse) <= sence) {
                Handles::s_Axes = Handles::POINT_B | Handles::POINT_L;
            } else if(HandleTools::distanceToPath(model, {tr, tl}, s_Mouse) <= sence) {
                Handles::s_Axes = Handles::POINT_T;
            } else if(HandleTools::distanceToPath(model, {br, bl}, s_Mouse) <= sence) {
                Handles::s_Axes = Handles::POINT_B;
            } else if(HandleTools::distanceToPath(model, {tr, br}, s_Mouse) <= sence) {
                Handles::s_Axes = Handles::POINT_R;
            } else if(HandleTools::distanceToPath(model, {tl, bl}, s_Mouse) <= sence) {
                Handles::s_Axes = Handles::POINT_L;
            } else {
                Ray ray = camera->castRay(Handles::s_Mouse.x, Handles::s_Mouse.y);
                if(ray.intersect(model * tr, model * tl, model * bl, nullptr, true) ||
                   ray.intersect(model * bl, model * br, model * tr, nullptr, true)) {
                    Handles::s_Axes = Handles::POINT_B | Handles::POINT_T | Handles::POINT_L | Handles::POINT_R;
                }
            }
        }

        Ray ray = camera->castRay(Handles::s_Mouse.x, Handles::s_Mouse.y);

        Ray::Hit hit;
        ray.intersect(plane, &hit, true);
        if(Handles::s_Axes & Handles::POINT_L || Handles::s_Axes & Handles::POINT_R) {
            switch(axis) {
                case Handles::AXIS_X: result.z = hit.point.z; break;
                case Handles::AXIS_Y: result.x = hit.point.x; break;
                case Handles::AXIS_Z: result.x = hit.point.x; break;
                default: break;
            }
        }
        if(Handles::s_Axes & Handles::POINT_T || Handles::s_Axes & Handles::POINT_B) {
            switch(axis) {
                case Handles::AXIS_X: result.y = hit.point.y; break;
                case Handles::AXIS_Y: result.z = hit.point.z; break;
                case Handles::AXIS_Z: result.y = hit.point.y; break;
                default: break;
            }
        }
    }
    return result;
}
