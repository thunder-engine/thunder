#include "editor/viewport/handles.h"
#include "editor/viewport/handletools.h"

#include "commandbuffer.h"
#include "pipelinecontext.h"
#include "components/camera.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"

#define SIDES 32
#define ALPHA 0.3f

#define CONTROL_SIZE 90.0f

#define OVERRIDE "texture0"

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

float Handles::s_Sense = 0.02f;

static CommandBuffer *s_Buffer = nullptr;

static float length = 1.0f;
static float conesize = length / 5.0f;

static float s_AngleBegin = 0.0f;
static float s_AngleTotal = 0.0f;

Mesh *Handles::s_Cone = nullptr;
Mesh *Handles::s_Quad = nullptr;
Mesh *Handles::s_Sphere = nullptr;
Mesh *Handles::s_Lines = nullptr;

MaterialInstance *Handles::s_Gizmo = nullptr;
MaterialInstance *Handles::s_Solid = nullptr;
MaterialInstance *Handles::s_Sprite = nullptr;

Mesh *Handles::s_Axis = nullptr;
Mesh *Handles::s_Scale = nullptr;
Mesh *Handles::s_ScaleXY = nullptr;
Mesh *Handles::s_ScaleXYZ = nullptr;
Mesh *Handles::s_Move = nullptr;
Mesh *Handles::s_MoveXY = nullptr;
Mesh *Handles::s_Arc = nullptr;
Mesh *Handles::s_Circle = nullptr;
Mesh *Handles::s_Rectangle = nullptr;
Mesh *Handles::s_Box = nullptr;
Mesh *Handles::s_Bone = nullptr;

Texture *Handles::s_Corner = nullptr;

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
    if(s_Sphere == nullptr) {
        s_Sphere = Engine::loadResource<Mesh>(".embedded/sphere.fbx/Sphere001");
    }
    if(s_Bone == nullptr) {
        s_Bone = Engine::loadResource<Mesh>(".embedded/bone.fbx/Bone");
    }

    if(s_Sprite == nullptr) {
        Material *m = Engine::loadResource<Material>(".embedded/DefaultSprite.mtl");
        if(m) {
            MaterialInstance *inst = m->createInstance();
            s_Sprite = inst;
        }
    }
    if(s_Gizmo == nullptr) {
        Material *m = Engine::loadResource<Material>(".embedded/gizmo.shader");
        if(m) {
            s_Gizmo = m->createInstance();
        }
    }
    if(s_Solid == nullptr) {
        Material *m = Engine::loadResource<Material>(".embedded/solid.shader");
        if(m) {
            s_Solid = m->createInstance();
        }
    }

    if(s_Corner == nullptr) {
        s_Corner = Engine::loadResource<Texture>(".embedded/corner.png");
    }

    if(s_Lines == nullptr) {
        s_Lines = Engine::objectCreate<Mesh>("Lines");
        s_Lines->makeDynamic();
    }

    if(s_Axis == nullptr) {
        Lod lod;
        lod.setVertices({Vector3(0.0f), Vector3(0, 5, 0)});
        lod.setIndices({0, 1});

        s_Axis = Engine::objectCreate<Mesh>("Axis");
        s_Axis->setTopology(Mesh::Lines);
        s_Axis->addLod(&lod);
    }

    if(s_Scale == nullptr) {
        Lod lod;
        lod.setVertices({Vector3(0, 2, 0), Vector3(1, 1, 0), Vector3(0, 3, 0), Vector3(1.5, 1.5, 0)});
        lod.setIndices({0, 1, 2, 3});

        s_Scale = Engine::objectCreate<Mesh>("Scale");
        s_Scale->setTopology(Mesh::Lines);
        s_Scale->addLod(&lod);
    }

    if(s_ScaleXY == nullptr) {
        Lod lod;
        lod.setVertices({Vector3(0, 2, 0), Vector3(1, 1, 0), Vector3(0, 3, 0), Vector3(1.5, 1.5, 0)});
        lod.setIndices({0, 1, 2, 1, 3, 2});

        s_ScaleXY = Engine::objectCreate<Mesh>("ScaleXY");
        s_ScaleXY->setTopology(Mesh::Triangles);
        s_ScaleXY->addLod(&lod);
    }

    if(s_ScaleXYZ == nullptr) {
        Lod lod;
        lod.setVertices({Vector3(0, 2, 0), Vector3(0, 0, 0), Vector3(1, 1, 0)});
        lod.setIndices({0, 1, 2});

        s_ScaleXYZ = Engine::objectCreate<Mesh>("ScaleXYZ");
        s_ScaleXYZ->setTopology(Mesh::Triangles);
        s_ScaleXYZ->addLod(&lod);
    }

    if(s_Move == nullptr) {
        Lod lod;
        lod.setVertices({Vector3(0, 1, 0), Vector3(2, 1, 0)});
        lod.setIndices({0, 1});

        s_Move = Engine::objectCreate<Mesh>("Move");
        s_Move->setTopology(Mesh::Lines);
        s_Move->addLod(&lod);
    }

    if(s_MoveXY == nullptr) {
        Lod lod;
        lod.setVertices({Vector3(0,-1, 0), Vector3(2,-1, 0), Vector3(0, 1, 0), Vector3(2, 1, 0)});
        lod.setIndices({0, 1, 2, 1, 3, 2});

        s_MoveXY = Engine::objectCreate<Mesh>("MoveXY");
        s_MoveXY->setTopology(Mesh::Triangles);
        s_MoveXY->addLod(&lod);
    }

    if(s_Arc == nullptr) {
        Lod lod;
        lod.setVertices(HandleTools::pointsArc(Quaternion(), 1.0, 0, 180));
        lod.indices().clear();

        s_Arc = Engine::objectCreate<Mesh>("Arc");
        s_Arc->setTopology(Mesh::LineStrip);
        s_Arc->addLod(&lod);
    }

    if(s_Circle == nullptr) {
        Lod lod;
        lod.setVertices(HandleTools::pointsArc(Quaternion(), 1.0, 0, 360));
        lod.indices().clear();

        s_Circle = Engine::objectCreate<Mesh>("Circle");
        s_Circle->setTopology(Mesh::LineStrip);
        s_Circle->addLod(&lod);
    }

    if(s_Rectangle == nullptr) {
        Lod lod;

        Vector3 min(-0.5);
        Vector3 max( 0.5);

        lod.setVertices({
            Vector3(min.x, min.y, 0.0f),
            Vector3(max.x, min.y, 0.0f),
            Vector3(max.x, max.y, 0.0f),
            Vector3(min.x, max.y, 0.0f),
            Vector3(min.x, min.y, 0.0f)
        });
        lod.indices().clear();

        s_Rectangle = Engine::objectCreate<Mesh>("Rectangle");
        s_Rectangle->setTopology(Mesh::LineStrip);
        s_Rectangle->addLod(&lod);
    }

    if(s_Box == nullptr) {
        Lod lod;

        Vector3 min(-0.5);
        Vector3 max( 0.5);

        lod.setVertices({
            Vector3(min.x, min.y, min.z),
            Vector3(max.x, min.y, min.z),
            Vector3(max.x, min.y, max.z),
            Vector3(min.x, min.y, max.z),

            Vector3(min.x, max.y, min.z),
            Vector3(max.x, max.y, min.z),
            Vector3(max.x, max.y, max.z),
            Vector3(min.x, max.y, max.z)
        });
        lod.setIndices({0, 1, 1, 2, 2, 3, 3, 0,
                        4, 5, 5, 6, 6, 7, 7, 4,
                        0, 4, 1, 5, 2, 6, 3, 7});

        s_Box = Engine::objectCreate<Mesh>("Box");
        s_Box->setTopology(Mesh::Lines);
        s_Box->addLod(&lod);
    }
}

void Handles::beginDraw(CommandBuffer *buffer) {
    if(CommandBuffer::isInited()) {
        Matrix4 v, p;
        Camera *cam = Camera::current();
        if(cam) {
            v = cam->viewMatrix();
            p = cam->projectionMatrix();
        }

        s_Buffer = buffer;

        HandleTools::setViewProjection(v, p);
        s_Buffer->setColor(s_Normal);
        s_Buffer->setViewProjection(v, p);
    }
}

void Handles::endDraw() {
    if(CommandBuffer::isInited() && s_Buffer) {
        s_Buffer->setColor(s_Normal);
    }
}

void Handles::drawArrow(const Matrix4 &transform) {
    if(CommandBuffer::isInited()) {
        s_Buffer->setColor(s_Color);
        s_Buffer->drawMesh(transform, s_Axis, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

        Matrix4 m;
        m.translate(Vector3(0, 4.0, 0));
        s_Buffer->setColor(s_Second);
        s_Buffer->drawMesh(transform * m, s_Cone, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
    }
}

void Handles::drawLines(const Matrix4 &transform, const Vector3Vector &points, const IndexVector &indices) {
    if(CommandBuffer::isInited()) {
        Lod lod;
        lod.setVertices(points);
        lod.setIndices(indices);
        {
            s_Lines->setTopology(Mesh::Lines);
            s_Lines->setLod(0, &lod);
        }
        if(s_Buffer) {
            s_Buffer->setColor(s_Color);
            s_Buffer->drawMesh(transform, s_Lines, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
        }
    }
}

void Handles::drawBox(const Vector3 &center, const Quaternion &rotation, const Vector3 &size) {
    if(CommandBuffer::isInited()) {
        s_Buffer->setColor(s_Color);

        Matrix4 transform(center, rotation, size);

        s_Buffer->drawMesh(transform, s_Box, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
    }
}

Vector3Vector Handles::makeCurve(const Vector3 &startPosition, const Vector3 &endPosition, const Vector3 &startTangent, const Vector3 &endTangent, int steps) {
    Vector3Vector points;
    points.resize(steps);
    for(int i = 0; i < steps; i++) {
        points[i] = CMIX(startPosition, startTangent, endTangent, endPosition, (float)i / float(steps-1));
    }
    return points;
}

void Handles::drawBone(const Transform *begin, const Transform *end) {
    if(CommandBuffer::isInited()) {
        s_Buffer->setColor(s_Color);

        Vector3 p0 = begin->worldPosition();
        Vector3 p1 = end->worldPosition();

        float size = (p1 - p0).length() * 0.1f;
        Matrix4 b(p0, begin->worldQuaternion(), Vector3(size));

        s_Buffer->drawMesh(b, s_Sphere, 0, CommandBuffer::TRANSLUCENT, s_Solid);
        s_Buffer->drawMesh(b, s_Bone, 0, CommandBuffer::TRANSLUCENT, s_Solid);
    }
}

void Handles::drawDisk(const Vector3 &center, const Quaternion &rotation, float radius, float start, float angle) {
    if(CommandBuffer::isInited()) {
        Lod lod;
        lod.setVertices(HandleTools::pointsArc(rotation, radius, start, angle, true));
        {
            s_Lines->setTopology(Mesh::TriangleFan);
            s_Lines->setLod(0, &lod);
        }
        if(s_Buffer) {
            Matrix4 transform;
            transform.translate(center);

            s_Buffer->setColor(s_Color);
            s_Buffer->drawMesh(transform, s_Lines, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
        }
    }
}

void Handles::drawRectangle(const Vector3 &center, const Quaternion &rotation, float width, float height) {
    if(CommandBuffer::isInited()) {
        s_Buffer->setColor(s_Color);

        Matrix4 transform(center, rotation, Vector3(width, height, 0.0f));

        s_Buffer->drawMesh(transform, s_Rectangle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
    }
}

void Handles::drawCircle(const Vector3 &center, const Quaternion &rotation, float radius) {
    if(CommandBuffer::isInited()) {
        s_Buffer->setColor(s_Color);

        Matrix4 transform(center, rotation, Vector3(radius));

        s_Buffer->drawMesh(transform, s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
    }
}

void Handles::drawSphere(const Vector3 &center, const Quaternion &rotation, float radius) {
    if(CommandBuffer::isInited()) {
        s_Buffer->setColor(s_Color);

        Matrix4 transform(center, rotation, Vector3(radius));

        s_Buffer->drawMesh(transform, s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
        s_Buffer->drawMesh(transform * Matrix4(Quaternion(Vector3(1, 0, 0), 90).toMatrix()), s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
        s_Buffer->drawMesh(transform * Matrix4(Quaternion(Vector3(0, 0, 1), 90).toMatrix()), s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
    }
}

void Handles::drawCapsule(const Vector3 &center, const Quaternion &rotation, float radius, float height) {
    if(CommandBuffer::isInited()) {
        s_Buffer->setColor(s_Color);

        Matrix4 transform(center, rotation, Vector3(1.0));

        float half = height * 0.5f - radius;

        {
            Vector3 cap(0, half, 0);
            s_Buffer->drawMesh(transform * Matrix4(cap, Quaternion(), Vector3(radius)), s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            s_Buffer->drawMesh(transform * Matrix4(cap, Quaternion(Vector3(-90,  0, 0)), Vector3(radius)), s_Arc, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            s_Buffer->drawMesh(transform * Matrix4(cap, Quaternion(Vector3(-90, 90, 0)), Vector3(radius)), s_Arc, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
        }
        {
            Vector3 cap(0,-half, 0);
            s_Buffer->drawMesh(transform * Matrix4(cap, Quaternion(), Vector3(radius)), s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            s_Buffer->drawMesh(transform * Matrix4(cap, Quaternion(Vector3( 90,  0, 0)), Vector3(radius)), s_Arc, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            s_Buffer->drawMesh(transform * Matrix4(cap, Quaternion(Vector3( 90, 90, 0)), Vector3(radius)), s_Arc, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
        }

        Vector3Vector points = { Vector3( radius, half, 0),
                                 Vector3( radius,-half, 0),
                                 Vector3(-radius, half, 0),
                                 Vector3(-radius,-half, 0),
                                 Vector3( 0, half, radius),
                                 Vector3( 0,-half, radius),
                                 Vector3( 0, half,-radius),
                                 Vector3( 0,-half,-radius)};

        IndexVector indices = {0, 1, 2, 3, 4, 5, 6, 7};

        drawLines(transform, points, indices);
    }
}

bool Handles::drawBillboard(const Vector3 &position, const Vector2 &size, Texture *texture) {
    bool result = false;
    if(CommandBuffer::isInited()) {
        Matrix4 model(position, Quaternion(), Vector3(size, size.x));
        Matrix4 q = model * Matrix4(Camera::current()->actor()->transform()->quaternion().toMatrix());

        if(HandleTools::distanceToPoint(q, Vector3()) <= s_Sense) {
            result = true;
        }

        s_Sprite->setTexture(OVERRIDE, texture);
        if(s_Buffer) {
            s_Buffer->setColor(s_Color);
            s_Buffer->drawMesh(q, PipelineContext::defaultPlane(), 0, CommandBuffer::TRANSLUCENT, s_Sprite);
        }
    }
    return result;
}

Vector3 Handles::moveTool(const Vector3 &position, const Quaternion &rotation, bool locked) {
    Vector3 result = position;
    if(CommandBuffer::isInited()) {
        Camera *camera = Camera::current();
        if(camera) {
            Vector3 normal = position - camera->actor()->transform()->position();
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
                Lod *move = s_Move->lod(0);
                Lod *axis = s_Axis->lod(0);
                if(HandleTools::distanceToPoint(model, Vector3()) <= s_Sense) {
                    s_Axes = AXIS_X | AXIS_Y | AXIS_Z;
                } else if((HandleTools::distanceToMesh(x, move->indices(), move->vertices()) <= s_Sense) ||
                          (HandleTools::distanceToMesh(z * r, move->indices(), move->vertices()) <= s_Sense)) {
                    s_Axes = AXIS_X | AXIS_Z;
                } else if((HandleTools::distanceToMesh(y, move->indices(), move->vertices()) <= s_Sense) ||
                          (HandleTools::distanceToMesh(x * r, move->indices(), move->vertices()) <= s_Sense)) {
                    s_Axes = AXIS_Y | AXIS_X;
                } else if((HandleTools::distanceToMesh(z, move->indices(), move->vertices()) <= s_Sense) ||
                          (HandleTools::distanceToMesh(y * r, move->indices(), move->vertices()) <= s_Sense)) {
                    s_Axes = AXIS_Z | AXIS_Y;
                } else if(HandleTools::distanceToMesh(x, axis->indices(), axis->vertices()) <= s_Sense) {
                    s_Axes = AXIS_X;
                } else if(HandleTools::distanceToMesh(y, axis->indices(), axis->vertices()) <= s_Sense) {
                    s_Axes = AXIS_Y;
                } else if(HandleTools::distanceToMesh(z, axis->indices(), axis->vertices()) <= s_Sense) {
                    s_Axes = AXIS_Z;
                }
            }

            s_Second = s_xColor;
            s_Color = (s_Axes & AXIS_X) ? s_Selected : s_xColor;
            drawArrow(x);
            s_Buffer->setColor(s_Axes == (AXIS_X | AXIS_Z) ? s_Selected : s_xColor);
            s_Buffer->drawMesh(x, s_Move, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            s_Buffer->setColor(s_Axes == (AXIS_X | AXIS_Y) ? s_Selected : s_xColor);
            s_Buffer->drawMesh(x * r, s_Move, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

            s_Second = s_yColor;
            s_Color = (s_Axes & AXIS_Y) ? s_Selected : s_yColor;
            drawArrow(y);
            s_Buffer->setColor(s_Axes == (AXIS_X | AXIS_Y) ? s_Selected : s_yColor);
            s_Buffer->drawMesh(y, s_Move, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            s_Buffer->setColor(s_Axes == (AXIS_Y | AXIS_Z) ? s_Selected : s_yColor);
            s_Buffer->drawMesh(y * r, s_Move, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

            s_Second = s_zColor;
            s_Color = (s_Axes & AXIS_Z) ? s_Selected : s_zColor;
            drawArrow(z);
            s_Buffer->setColor(s_Axes == (AXIS_Y | AXIS_Z) ? s_Selected : s_zColor);
            s_Buffer->drawMesh(z, s_Move, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            s_Buffer->setColor(s_Axes == (AXIS_X | AXIS_Z) ? s_Selected : s_zColor);
            s_Buffer->drawMesh(z * r, s_Move, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

            s_Color = s_Selected;
            s_Color.w = ALPHA;
            if(s_Axes == (AXIS_X | AXIS_Z)) {
                s_Buffer->setColor(s_Color);
                s_Buffer->drawMesh(x, s_MoveXY, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            }
            if(s_Axes == (AXIS_X | AXIS_Y)) {
                s_Buffer->setColor(s_Color);
                s_Buffer->drawMesh(y, s_MoveXY, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            }
            if(s_Axes == (AXIS_Y | AXIS_Z)) {
                s_Buffer->setColor(s_Color);
                s_Buffer->drawMesh(z, s_MoveXY, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            }
            s_Color = s_Normal;
            s_Second = s_Normal;

            Plane plane;
            plane.point = position;
            plane.normal = camera->actor()->transform()->quaternion() * Vector3(0.0f, 0.0f, 1.0f);
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
            Vector3 point;
            ray.intersect(plane, &point, true);

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

            result = point * mask;
        }
    }
    return result;
}

float Handles::rotationTool(const Vector3 &position, const Quaternion &rotation, bool locked) {
    if(CommandBuffer::isInited()) {
        Camera *camera = Camera::current();
        if(camera) {
            Transform *t = camera->actor()->transform();
            Vector3 normal = position - t->position();
            float scale = 1.0f;
            if(!camera->orthographic()) {
                scale = normal.length();
            } else {
                scale = camera->orthoSize();
            }
            scale *= (CONTROL_SIZE / s_Screen.y);
            normal.normalize();

            Matrix4 model(position, rotation, scale * 5.0f);

            Matrix4 q1 = Matrix4(position, Quaternion(), scale * 5.0f) *
                         Matrix4(Vector3(), t->quaternion() * Quaternion(Vector3(90, 0, 0)), Vector3(conesize));

            Matrix4 x = model * Matrix4(Vector3(), Quaternion(Vector3( 0, 0, 90)) *
                                                   Quaternion(Vector3( 0, 1, 0), RAD2DEG * atan2(normal.y, normal.z) + 180), Vector3(conesize));
            Matrix4 y = model * Matrix4(Vector3(), Quaternion(Vector3( 0, 1, 0), RAD2DEG * atan2(normal.x, normal.z) + 180), Vector3(conesize));
            Matrix4 z = model * Matrix4(Vector3(), Quaternion(Vector3( 0, 0, 1),-RAD2DEG * atan2(normal.x, normal.y)) *
                                                   Quaternion(Vector3(90, 0, 0)), Vector3(conesize));

            Matrix4 m;
            m.scale(1.2f);

            if(!locked) {
                Lod *circle = s_Circle->lod(0);
                Lod *arc = s_Arc->lod(0);
                if(HandleTools::distanceToMesh(q1 * m, circle->indices(), circle->vertices()) <= s_Sense) {
                    s_Axes = AXIS_X | AXIS_Y | AXIS_Z;
                } else if(HandleTools::distanceToMesh(x, arc->indices(), arc->vertices()) <= s_Sense) {
                    s_Axes = AXIS_X;
                } else if(HandleTools::distanceToMesh(y, arc->indices(), arc->vertices()) <= s_Sense) {
                    s_Axes = AXIS_Y;
                } else if(HandleTools::distanceToMesh(z, arc->indices(), arc->vertices()) <= s_Sense) {
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

            Vector3 world;
            Ray ray = camera->castRay(s_Mouse.x, s_Mouse.y);
            ray.intersect(plane, &world, true);

            Vector3 dt0 = world - position;
            if(locked) {
                Vector3 dt1 = s_World - position;
                float angle = dt1.signedAngle(dt0, plane.normal);

                s_AngleTotal += angle;
                s_Color = s_Selected;
                s_Color.w = ALPHA;
                if(s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) {
                    drawDisk(position, t->quaternion() * Quaternion(Vector3(90, 0, 0)), scale, s_AngleBegin, -s_AngleTotal);
                    s_Buffer->setColor(s_Selected);
                    s_Buffer->drawMesh(q1 * m, s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                } else if(s_Axes == AXIS_X) {
                    drawDisk(position, Quaternion(Vector3(0, 0, 1), 90), scale, s_AngleBegin, s_AngleTotal);
                    s_Buffer->setColor(s_Selected);
                    s_Buffer->drawMesh(x, s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                } else if(s_Axes == AXIS_Y) {
                    drawDisk(position, Quaternion(), scale, s_AngleBegin, -s_AngleTotal);
                    s_Buffer->setColor(s_Selected);
                    s_Buffer->drawMesh(y, s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                } else if(s_Axes == AXIS_Z) {
                    drawDisk(position, Quaternion(Vector3(1, 0, 0), 90), scale, s_AngleBegin, -s_AngleTotal);
                    s_Buffer->setColor(s_Selected);
                    s_Buffer->drawMesh(z, s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                }
            } else {
                s_Color = (s_Axes == AXIS_X) ? s_Selected : s_xColor;
                s_Buffer->setColor(s_Color);
                s_Buffer->drawMesh(x, camera->orthographic() ? s_Circle : s_Arc, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

                s_Color = (s_Axes == AXIS_Y) ? s_Selected : s_yColor;
                s_Buffer->setColor(s_Color);
                s_Buffer->drawMesh(y, camera->orthographic() ? s_Circle : s_Arc, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

                s_Color = (s_Axes == AXIS_Z) ? s_Selected : s_zColor;
                s_Buffer->setColor(s_Color);
                s_Buffer->drawMesh(z, camera->orthographic() ? s_Circle : s_Arc, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

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
            s_Buffer->setColor(s_Color);
            s_Buffer->drawMesh(q1 * m, s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            s_Buffer->setColor(s_Grey);
            s_Buffer->drawMesh(q1, s_Circle, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

            s_Color = s_Normal;
            s_Buffer->setColor(s_Color);

            s_World = world;
        }
    }
    return s_AngleTotal;
}

Vector3 Handles::scaleTool(const Vector3 &position, const Quaternion &rotation, bool locked) {
    Vector3 result;
    if(CommandBuffer::isInited()) {
        Camera *camera = Camera::current();
        if(camera) {
            Vector3 normal = position - camera->actor()->transform()->position();
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
                Lod *scale = s_Scale->lod(0);
                Lod *axis = s_Axis->lod(0);

                if(HandleTools::distanceToPoint(model, Vector3()) <= s_Sense) {
                    s_Axes = AXIS_X | AXIS_Y | AXIS_Z;
                } else if((HandleTools::distanceToMesh(x, scale->indices(), scale->vertices()) <= s_Sense) ||
                          (HandleTools::distanceToMesh(z * r, scale->indices(), scale->vertices()) <= s_Sense)) {
                    s_Axes = AXIS_X | AXIS_Z;
                } else if((HandleTools::distanceToMesh(y, scale->indices(), scale->vertices()) <= s_Sense) ||
                          (HandleTools::distanceToMesh(x * r, scale->indices(), scale->vertices()) <= s_Sense)) {
                    s_Axes = AXIS_Y | AXIS_X;
                } else if((HandleTools::distanceToMesh(z, scale->indices(), scale->vertices()) <= s_Sense) ||
                          (HandleTools::distanceToMesh(y * r, scale->indices(), scale->vertices()) <= s_Sense)) {
                    s_Axes = AXIS_Z | AXIS_Y;
                } else if(HandleTools::distanceToMesh(x, axis->indices(), axis->vertices()) <= s_Sense) {
                    s_Axes = AXIS_X;
                } else if(HandleTools::distanceToMesh(y, axis->indices(), axis->vertices()) <= s_Sense) {
                    s_Axes = AXIS_Y;
                } else if(HandleTools::distanceToMesh(z, axis->indices(), axis->vertices()) <= s_Sense) {
                    s_Axes = AXIS_Z;
                }
            }

            s_Color = s_Selected;
            s_Color.w = ALPHA;
            {
                s_Buffer->setColor(s_xColor);
                if(s_Axes == (AXIS_X | AXIS_Z)) {
                    s_Buffer->setColor(s_Color);
                    s_Buffer->drawMesh(x, s_ScaleXY, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                    s_Buffer->setColor(s_Selected);
                }
                s_Buffer->drawMesh(x, s_Scale, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

                s_Buffer->setColor(s_xColor);
                if(s_Axes == (AXIS_X | AXIS_Y)) {
                    s_Buffer->setColor(s_Color);
                    s_Buffer->drawMesh(x * r, s_ScaleXY, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                    s_Buffer->setColor(s_Selected);
                }
                s_Buffer->drawMesh(x * r, s_Scale, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

                s_Buffer->setColor((s_Axes & AXIS_X) ? s_Selected : s_xColor);
                s_Buffer->drawMesh(x, s_Axis, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            }
            {
                s_Buffer->setColor(s_yColor);
                if(s_Axes == (AXIS_Y | AXIS_X)) {
                    s_Buffer->setColor(s_Color);
                    s_Buffer->drawMesh(y, s_ScaleXY, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                    s_Buffer->setColor(s_Selected);
                }
                s_Buffer->drawMesh(y, s_Scale, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

                s_Buffer->setColor(s_yColor);
                if(s_Axes == (AXIS_Y | AXIS_Z)) {
                    s_Buffer->setColor(s_Color);
                    s_Buffer->drawMesh(y * r, s_ScaleXY, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                    s_Buffer->setColor(s_Selected);
                }
                s_Buffer->drawMesh(y * r, s_Scale, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

                s_Buffer->setColor((s_Axes & AXIS_Y) ? s_Selected : s_yColor);
                s_Buffer->drawMesh(y, s_Axis, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            }
            {
                s_Buffer->setColor(s_zColor);
                if(s_Axes == (AXIS_Z | AXIS_Y)) {
                    s_Buffer->setColor(s_Color);
                    s_Buffer->drawMesh(z, s_ScaleXY, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                    s_Buffer->setColor(s_Selected);
                }
                s_Buffer->drawMesh(z, s_Scale, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

                s_Buffer->setColor(s_zColor);
                if(s_Axes == (AXIS_Z | AXIS_X)) {
                    s_Buffer->setColor(s_Color);
                    s_Buffer->drawMesh(z * r, s_ScaleXY, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                    s_Buffer->setColor(s_Selected);
                }
                s_Buffer->drawMesh(z * r, s_Scale, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);

                s_Buffer->setColor((s_Axes & AXIS_Z) ? s_Selected : s_zColor);
                s_Buffer->drawMesh(z, s_Axis, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            }

            if(s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) {
                s_Buffer->setColor(s_Color);
                s_Buffer->drawMesh(x,     s_ScaleXYZ, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                s_Buffer->drawMesh(x * r, s_ScaleXYZ, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                s_Buffer->drawMesh(y,     s_ScaleXYZ, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                s_Buffer->drawMesh(y * r, s_ScaleXYZ, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                s_Buffer->drawMesh(z,     s_ScaleXYZ, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
                s_Buffer->drawMesh(z * r, s_ScaleXYZ, 0, CommandBuffer::TRANSLUCENT, s_Gizmo);
            }
            s_Color = s_Normal;

            Plane plane;
            plane.point = position;
            plane.normal = camera->actor()->transform()->quaternion() * Vector3(0.0f, 0.0f, 1.0f);
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
            Vector3 point;
            ray.intersect(plane, &point, true);
            if(s_Axes & AXIS_X) {
                result.x = point.x;
            }
            if(s_Axes & AXIS_Y) {
                result.y = point.y;
            }
            if(s_Axes & AXIS_Z) {
                result.z = point.z;
            }
        }
    }
    return result;
}

Vector3 Handles::rectTool(const Vector3 &position, const Vector3 &box, int &axis, bool locked) {
    Vector3 result;
    if(CommandBuffer::isInited()) {
        Camera *camera = Camera::current();

        if(camera) {
            axis = Handles::AXIS_Z;

            Plane plane;
            plane.normal = Vector3(0.0f, 0.0f, 1.0f);

            Vector2 size(box.x, box.y);

            Quaternion q;

            Vector3 normal = camera->actor()->transform()->quaternion() * plane.normal;
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

            drawRectangle(position, q, size.x, size.y);

            Transform *t = camera->actor()->transform();
            normal = position - t->position();
            float scale = 1.0f;
            if(!camera->orthographic()) {
                scale = normal.length();
            } else {
                scale = camera->orthoSize();
            }
            scale *= (CONTROL_SIZE / s_Screen.y);

            drawBillboard(model * tr, Vector2(scale * 0.05f), s_Corner);
            drawBillboard(model * tl, Vector2(scale * 0.05f), s_Corner);
            drawBillboard(model * br, Vector2(scale * 0.05f), s_Corner);
            drawBillboard(model * bl, Vector2(scale * 0.05f), s_Corner);

            if(!locked) {
                float sence = Handles::s_Sense * 0.25f;

                Handles::s_Axes = 0;
                if(HandleTools::distanceToPoint(model, tr) <= sence) {
                    Handles::s_Axes = Handles::POINT_T | Handles::POINT_R;
                } else if(HandleTools::distanceToPoint(model, tl) <= sence) {
                    Handles::s_Axes = Handles::POINT_T | Handles::POINT_L;
                } else if(HandleTools::distanceToPoint(model, br) <= sence) {
                    Handles::s_Axes = Handles::POINT_B | Handles::POINT_R;
                } else if(HandleTools::distanceToPoint(model, bl) <= sence) {
                    Handles::s_Axes = Handles::POINT_B | Handles::POINT_L;
                } else if(HandleTools::distanceToPath(model, {tr, tl}) <= sence) {
                    Handles::s_Axes = Handles::POINT_T;
                } else if(HandleTools::distanceToPath(model, {br, bl}) <= sence) {
                    Handles::s_Axes = Handles::POINT_B;
                } else if(HandleTools::distanceToPath(model, {tr, br}) <= sence) {
                    Handles::s_Axes = Handles::POINT_R;
                } else if(HandleTools::distanceToPath(model, {tl, bl}) <= sence) {
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
            Vector3 point;

            ray.intersect(plane, &point, true);
            if(Handles::s_Axes & Handles::POINT_L || Handles::s_Axes & Handles::POINT_R) {
                switch(axis) {
                    case Handles::AXIS_X: result.z = point.z; break;
                    case Handles::AXIS_Y: result.x = point.x; break;
                    case Handles::AXIS_Z: result.x = point.x; break;
                    default: break;
                }
            }
            if(Handles::s_Axes & Handles::POINT_T || Handles::s_Axes & Handles::POINT_B) {
                switch(axis) {
                    case Handles::AXIS_X: result.y = point.y; break;
                    case Handles::AXIS_Y: result.z = point.z; break;
                    case Handles::AXIS_Z: result.y = point.y; break;
                    default: break;
                }
            }
        }
    }
    return result;
}
