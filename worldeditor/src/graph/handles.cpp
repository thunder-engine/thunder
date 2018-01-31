#include "handles.h"

#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#include <rendersystem.h>
#include <components/camera.h>
#include <components/actor.h>

#include "handletools.h"

#define SIDES 32

Vector4 Handles::s_Normal   = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
Vector4 Handles::s_Selected = Vector4(1.0f, 1.0f, 0.0f, 1.0f);
Vector4 Handles::s_xColor   = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
Vector4 Handles::s_yColor   = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
Vector4 Handles::s_zColor   = Vector4(0.0f, 0.0f, 1.0f, 1.0f);

Vector4 Handles::s_Color    = Handles::s_Normal;
Vector4 Handles::s_Second   = Handles::s_Normal;

Vector3 Handles::m_sMouse   = Vector3();

const Vector4 grey(0.3, 0.3, 0.3, 1.0);

Camera *Handles::s_ActiveCamera = nullptr;

uint8_t Handles::s_Axes     = 0;

static IRenderSystem *s_System  = nullptr;

float length    = 0.7f;
float sense     = 10.0f;

void *s_Quadric    = gluNewQuadric();

void Handles::init(IRenderSystem *system) {
    s_System  = system;
}

void Handles::beginDraw() {
    Matrix4 v, p;
    if(s_ActiveCamera) {
        s_ActiveCamera->matrices(v, p);
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(p.mat);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(v.mat);

    glEnable        (GL_BLEND);
    glBlendFunc     (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable       (GL_DEPTH_TEST);
}

void Handles::endDraw() {
    glDisable       (GL_BLEND);

    glEnable        (GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void Handles::drawArrow(const Vector3 &position, const Quaternion &rotation, float size) {
    Matrix4 transform   = HandleTools::modelView() * Matrix4(position, rotation, 1.0f);
    glPushMatrix();
    glLoadMatrixf(transform.mat);

    float conesize  = size / 5.0f;

    s_System->setColor(s_Color);
    s_System->drawStrip(transform, {Vector3(0.0f), Vector3(0, 0, size)}, true);
    s_System->setColor(s_Second);
    drawCone(Vector3(0, 0, size - conesize), Quaternion (), conesize);

    glPopMatrix();
}

void Handles::drawCone(const Vector3 &position, const Quaternion &rotation, float size) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glMultMatrixf(Matrix4(rotation.toMatrix()).mat);

    gluCylinder((GLUquadric *)s_Quadric, size / 4.0f, 0.0f, size, 8, 1);

    glPopMatrix();
}

void Handles::drawDisk(const Vector3 &position, const Quaternion &rotation, float size, float start, float angle) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glMultMatrixf(Matrix4(rotation.toMatrix()).mat);
    glRotatef(90, 1, 0, 0);
    s_System->setColor(s_Color);
    gluPartialDisk((GLUquadric *)s_Quadric, 0.0, size, SIDES, 1, start, angle);

    glPopMatrix();
}

void Handles::drawQuad(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &p4) {
    s_System->setColor(s_Color);

    Vector3List points = {p1, p2, p3, p4};

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &points[0]);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, points.size());

    glDisableClientState(GL_VERTEX_ARRAY);
}

void Handles::drawFrustum(const Vector3List &points) {
    s_System->setColor(s_Color);

    GLubyte indices[] = {0, 1, 5, 4,
                         3, 0, 4, 7,
                         2, 3, 7, 6,
                         1, 2, 6, 5};

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &points[0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_QUADS, 16, GL_UNSIGNED_BYTE, indices);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDisableClientState(GL_VERTEX_ARRAY);
}

void Handles::drawBox(const Vector3 &position, const Quaternion &rotation, const Vector3 &size) {
    Vector3 min = position - size * 0.5;
    Vector3 max = min + size;

    Vector3List vertices;
    vertices.push_back(Vector3(min.x, min.y, min.z));
    vertices.push_back(Vector3(min.x, min.y, max.z));
    vertices.push_back(Vector3(max.x, min.y, max.z));
    vertices.push_back(Vector3(max.x, min.y, min.z));

    vertices.push_back(Vector3(min.x, max.y, min.z));
    vertices.push_back(Vector3(min.x, max.y, max.z));
    vertices.push_back(Vector3(max.x, max.y, max.z));
    vertices.push_back(Vector3(max.x, max.y, min.z));

    drawFrustum(vertices);
}

void Handles::drawBillboard(const Vector3 &position, const Vector2 &size, Texture &texture) {
    //s_System->drawBillboard(position, size, texture);
}

Vector3 Handles::moveTool(const Vector3 &position, bool locked) {
    Vector3 result    = position;
    if(s_ActiveCamera) {
        float scale   = (position - s_ActiveCamera->actor().position()).length() * cos(s_ActiveCamera->fov() * DEG2RAD) * 0.2f;
        Matrix4 model(position, Quaternion(), scale);

        Matrix4 v, p;
        s_ActiveCamera->matrices(v, p);

        HandleTools::pushCamera(*s_ActiveCamera, model);

        glPushMatrix();
        glLoadMatrixf((v * model).mat);

        float conesize  = length / 5.0f;
        float consize2  = conesize * 2.0f;

        Vector3 x( length, 0.0f, 0.0f);
        Vector3 y( 0.0f, length, 0.0f);
        Vector3 z( 0.0f, 0.0f, length);

        Vector3 xy( consize2, consize2, 0.0f);
        Vector3 yz( 0.0f, consize2, consize2);
        Vector3 xz( consize2, 0.0f, consize2);

        if(!locked) {
            s_Axes  = AXIS_X | AXIS_Y | AXIS_Z;
            if(HandleTools::distanceToPoint(xy) <= sense * 2.0) {
                s_Axes  = AXIS_X | AXIS_Y;
            } else if(HandleTools::distanceToPoint(yz) <= sense * 2.0) {
                s_Axes  = AXIS_Y | AXIS_Z;
            } else if(HandleTools::distanceToPoint(xz) <= sense * 2.0) {
                s_Axes  = AXIS_X | AXIS_Z;
            } else if(HandleTools::distanceToPath({Vector3(), x}) <= sense) {
                s_Axes  = AXIS_X;
            } else if(HandleTools::distanceToPath({Vector3(), y}) <= sense) {
                s_Axes  = AXIS_Y;
            } else if(HandleTools::distanceToPath({Vector3(), z}) <= sense) {
                s_Axes  = AXIS_Z;
            }
        }

        Plane plane;
        plane.point     = position;
        plane.normal    = s_ActiveCamera->actor().rotation() * Vector3(0.0, 0.0, 1.0);
        if(s_Axes == AXIS_X || s_Axes == AXIS_Z) {
            plane.normal    = Vector3(0.0f, plane.normal.y, plane.normal.z);
        } else if(s_Axes == (AXIS_X | AXIS_Y)) {
            plane.normal    = Vector3(0.0f, 0.0f, 1.0f);
        } else if(s_Axes == (AXIS_Z | AXIS_Y)) {
            plane.normal    = Vector3(1.0f, 0.0f, 0.0f);
        } else if(s_Axes == AXIS_Y || s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) {
            plane.normal    = s_ActiveCamera->actor().rotation() * Vector3(0.0, 0.0, 1.0);
            plane.normal    = Vector3(plane.normal.x, 0.0f, plane.normal.z);
        }
        plane.d = plane.normal.dot(plane.point);

        Ray ray = s_ActiveCamera->castRay(m_sMouse.x / (float)s_ActiveCamera->width(),
                                          m_sMouse.y / (float)s_ActiveCamera->height());
        Vector3 point;
        ray.intersect(plane, &point, true);
        if(s_Axes & AXIS_X) {
            result.x    = point.x;
        }
        if(s_Axes & AXIS_Y) {
            result.y    = point.y;
        }
        if(s_Axes & AXIS_Z) {
            result.z    = point.z;
        }

        s_Second    = s_xColor;
        s_Color     = (s_Axes & AXIS_X) ? s_Selected : s_xColor;
        s_System->setColor(s_Axes == (AXIS_X | AXIS_Z) ? s_Selected : s_xColor);
        s_System->drawStrip(model, {Vector3(consize2, 0, 0), xz}, true);
        s_System->setColor(s_Axes == (AXIS_X | AXIS_Y) ? s_Selected : s_xColor);
        s_System->drawStrip(model, {Vector3(consize2, 0, 0), xy}, true);

        drawArrow(Vector3(conesize, 0, 0), Quaternion(Vector3(0, 1, 0), 90), length);

        s_Second    = s_yColor;
        s_Color     = (s_Axes & AXIS_Y) ? s_Selected : s_yColor;
        s_System->setColor(s_Axes == (AXIS_X | AXIS_Y) ? s_Selected : s_yColor);
        s_System->drawStrip(model, {Vector3(0, consize2, 0), xy}, true);
        s_System->setColor(s_Axes == (AXIS_Y | AXIS_Z) ? s_Selected : s_yColor);
        s_System->drawStrip(model, {Vector3(0, consize2, 0), yz}, true);
        drawArrow(Vector3(0, conesize, 0), Quaternion(Vector3(1, 0, 0),-90), length);

        s_Second    = s_zColor;
        s_Color     = (s_Axes & AXIS_Z) ? s_Selected : s_zColor;
        s_System->setColor(s_Axes == (AXIS_Y | AXIS_Z) ? s_Selected : s_zColor);
        s_System->drawStrip(model, {Vector3(0, 0, consize2), yz}, true);
        s_System->setColor(s_Axes == (AXIS_X | AXIS_Z) ? s_Selected : s_zColor);
        s_System->drawStrip(model, {Vector3(0, 0, consize2), xz}, true);
        drawArrow(Vector3(0, 0, conesize), Quaternion(Vector3(0, 0, 1), 90), length);

        s_Color = s_Selected;
        s_Color.w = 0.2f;
        if(s_Axes == (AXIS_X | AXIS_Z)) {
            drawQuad(Vector3(), Vector3(0.0f, xz.y, xz.z), Vector3(xz.x, xz.y, 0.0f), xz);
        }
        if(s_Axes == (AXIS_X | AXIS_Y)) {
            drawQuad(Vector3(), Vector3(0.0f, xy.y, xy.z), Vector3(xy.x, 0.0f, xy.z), xy);
        }
        if(s_Axes == (AXIS_Y | AXIS_Z)) {
            drawQuad(Vector3(), Vector3(yz.x, 0.0f, yz.z), Vector3(yz.x, yz.y, 0.0f), yz);
        }
        s_Color = s_Normal;

        HandleTools::popCamera();
        glPopMatrix();
    }

    return result;
}

Vector3 Handles::rotationTool(const Vector3 &position, bool locked) {
    if(s_ActiveCamera) {
        float half      = 180.0f;
        float scale     = ((position - s_ActiveCamera->actor().position()).length() * cos(s_ActiveCamera->fov() / 2 * DEG2RAD) * 0.2f) * length;
        Vector3 normal  = position - s_ActiveCamera->actor().position();
        normal.normalize();

        Matrix4 model;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        Quaternion q    = s_ActiveCamera->actor().rotation() * Quaternion(Vector3(90.0, 0, 0));

        Vector3List pb  = IRenderSystem::pointsArc(q, scale * 1.4f, 0, half * 2.0f);
        Vector3List px  = IRenderSystem::pointsArc(Quaternion(Vector3(0, 0, 1), 90),   scale,-RAD2DEG * atan2(normal.y, normal.z) + half, half);
        Vector3List py  = IRenderSystem::pointsArc(Quaternion(Vector3(0, 1, 0), half), scale,-RAD2DEG * atan2(normal.x, normal.z), half);
        Vector3List pz  = IRenderSystem::pointsArc(Quaternion(Vector3(1, 0, 0), 90),   scale, RAD2DEG * atan2(normal.x, normal.y), half);

        if(!locked) {
            HandleTools::pushCamera(*s_ActiveCamera, Matrix4());
            if(HandleTools::distanceToPath(pb) <= sense) {
                s_Axes  = AXIS_X | AXIS_Y | AXIS_Z;
            } else if(HandleTools::distanceToPath(px) <= sense) {
                s_Axes  = AXIS_X;
            } else if(HandleTools::distanceToPath(py) <= sense) {
                s_Axes  = AXIS_Y;
            } else if(HandleTools::distanceToPath(pz) <= sense) {
                s_Axes  = AXIS_Z;
            }
            HandleTools::popCamera();
        }

        s_System->setColor((s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) ? s_Selected : grey * 2.0f);
        s_System->drawStrip(model, pb, true);
        s_System->setColor(grey);
        s_System->drawStrip(model, IRenderSystem::pointsArc(q, scale, 0, half * 2.0f), true);

        if(!locked || s_Axes == AXIS_X) {
            s_System->setColor((s_Axes == AXIS_X) ? s_Selected : s_xColor);
            s_System->drawStrip(model, px, true);
        }

        if(!locked || s_Axes == AXIS_Y) {
            s_System->setColor((s_Axes == AXIS_Y) ? s_Selected : s_yColor);
            s_System->drawStrip(model, py, true);
        }

        if(!locked || s_Axes == AXIS_Z) {
            s_System->setColor((s_Axes == AXIS_Z) ? s_Selected : s_zColor);
            s_System->drawStrip(model, pz, true);
        }
        glPopMatrix();
/*
        if(locked) {
            Quaternion  r   = q;
            s_Color = grey * 2.0f;
            switch(s_Axes) {
                case AXIS_X: {
                    s_Color = s_xColor;
                    r   = Quaternion (Vector3( 0.0, 0.0,90.0));
                } break;
                case AXIS_Y: {
                    s_Color = s_yColor;
                    r   = Quaternion ();
                } break;
                case AXIS_Z: {
                    s_Color = s_zColor;
                    r   = Quaternion (Vector3(90.0, 0.0, 0.0));
                } break;
                default: break;
            }
            s_Color.w   = 0.2f;
            drawDisk(position, r, scale, 0.0f, value);
        }
*/
        s_System->setColor(s_Normal);
    }

    return m_sMouse;
}

Vector3 Handles::scaleTool(const Vector3 &position, bool locked) {
    if(s_ActiveCamera) {
        Vector3 normal  = position - s_ActiveCamera->actor().position();
        float size      = normal.length() * cos(s_ActiveCamera->fov() / 2 * DEG2RAD) * 0.2;

        Vector3 scale(((normal.x < 0) ? 1 : -1) * size,
                      ((normal.y > 0) ? 1 : -1) * size,
                      ((normal.z < 0) ? 1 : -1) * size);

        Matrix4 model(position, Quaternion(), scale);

        HandleTools::pushCamera(*s_ActiveCamera, model);

        Matrix4 v, p;
        s_ActiveCamera->matrices(v, p);

        glPushMatrix();
        glLoadMatrixf((v * model).mat);

        float half  = length * 0.5;
        float hh    = half * 0.5;
        float big   = half * 1.4;
        float hbig  = big  * 0.5;

        if(!locked) {
            Vector3 x0  = Vector3(half, 0, 0);
            Vector3 y0  = Vector3(0,-half, 0);
            Vector3 z0  = Vector3(0, 0, half);

            Vector3 x1  = Vector3(length, 0, 0);
            Vector3 y1  = Vector3(0,-length, 0);
            Vector3 z1  = Vector3(0, 0, length);

            if(HandleTools::distanceToPath({x0, y0}) <= sense) {
                s_Axes  = AXIS_X | AXIS_Y;
            } else if(HandleTools::distanceToPath({x0, z0}) <= sense) {
                s_Axes  = AXIS_X | AXIS_Z;
            } else if(HandleTools::distanceToPath({y0, z0}) <= sense) {
                s_Axes  = AXIS_Y | AXIS_Z;
            } else if(HandleTools::distanceToPath({x0, x1}) <= sense) {
                s_Axes  = AXIS_X;
            } else if(HandleTools::distanceToPath({y0, y1}) <= sense) {
                s_Axes  = AXIS_Y;
            } else if(HandleTools::distanceToPath({z0, z1}) <= sense) {
                s_Axes  = AXIS_Z;
            }
        }

        // X Axis
        glPushMatrix();
        glRotatef(90, 0, 1, 0);
        s_System->setColor(s_Axes == AXIS_X ? s_Selected : s_xColor);
        s_System->drawStrip(model, {Vector3(), Vector3(0, 0, length)}, true);
        s_System->setColor((s_Axes & AXIS_X && s_Axes & AXIS_Y) ? s_Selected : s_xColor);
        s_System->drawStrip(model, {Vector3(0, 0, half), Vector3(0,-hh,   hh)}, true);
        s_System->drawStrip(model, {Vector3(0, 0, big),  Vector3(0,-hbig, hbig)}, true);
        s_System->setColor((s_Axes & AXIS_X && s_Axes & AXIS_Z) ? s_Selected : s_xColor);
        s_System->drawStrip(model, {Vector3(0, 0, half), Vector3(-hh,   0, hh)}, true);
        s_System->drawStrip(model, {Vector3(0, 0, big),  Vector3(-hbig, 0, hbig)}, true);
        glPopMatrix();
        // Y Axis
        glPushMatrix();
        glRotatef(90, 1, 0, 0);
        s_System->setColor(s_Axes == AXIS_Y ? s_Selected : s_yColor);
        s_System->drawStrip(model, {Vector3(), Vector3(0, 0, length)}, true);
        s_System->setColor((s_Axes & AXIS_Y && s_Axes & AXIS_Z) ? s_Selected : s_yColor);
        s_System->drawStrip(model, {Vector3(0, 0, half), Vector3(0, hh,   hh)}, true);
        s_System->drawStrip(model, {Vector3(0, 0, big),  Vector3(0, hbig, hbig)}, true);
        s_System->setColor((s_Axes & AXIS_X && s_Axes & AXIS_Y) ? s_Selected : s_yColor);
        s_System->drawStrip(model, {Vector3(0, 0, half), Vector3(hh,   0,  hh)}, true);
        s_System->drawStrip(model, {Vector3(0, 0, big),  Vector3(hbig, 0, hbig)}, true);
        glPopMatrix();
        // Z Axis
        glPushMatrix();
        glRotatef(90, 0, 0, 1);
        s_System->setColor(s_Axes == AXIS_Z ? s_Selected : s_zColor);
        s_System->drawStrip(model, {Vector3(), Vector3(0, 0, length)}, true);
        s_System->setColor((s_Axes & AXIS_X && s_Axes & AXIS_Z) ? s_Selected : s_zColor);
        s_System->drawStrip(model, {Vector3(0, 0, half), Vector3(0,-hh,   hh)}, true);
        s_System->drawStrip(model, {Vector3(0, 0, big),  Vector3(0,-hbig, hbig)}, true);
        s_System->setColor((s_Axes & AXIS_Y && s_Axes & AXIS_Z) ? s_Selected : s_zColor);
        s_System->drawStrip(model, {Vector3(0, 0, half), Vector3(-hh,   0, hh)}, true);
        s_System->drawStrip(model, {Vector3(0, 0, big),  Vector3(-hbig, 0, hbig)}, true);
        glPopMatrix();

        s_Color     = s_Selected;
        s_Color.w   = 0.2f;
        s_System->setColor(s_Color);
        if(s_Axes == (AXIS_X | AXIS_Y)) {
            drawQuad(Vector3(half, 0, 0), Vector3(big, 0, 0), Vector3(0,-half, 0), Vector3(0,-big, 0));
        } else if(s_Axes == (AXIS_X | AXIS_Z)) {
            drawQuad(Vector3(half, 0, 0), Vector3(big, 0, 0), Vector3(0, 0, half), Vector3(0, 0, big));
        } else if(s_Axes == (AXIS_Y | AXIS_Z)) {
            drawQuad(Vector3(0,-half, 0), Vector3(0,-big, 0), Vector3(0, 0, half), Vector3(0, 0, big));
        } else if(s_Axes == (AXIS_X | AXIS_Y | AXIS_Z)) {
            glBegin(GL_TRIANGLE_STRIP);
                glVertex3f( half, 0, 0 );
                glVertex3f( 0,-half, 0 );
                glVertex3f( 0, 0, half );
            glEnd();
        }
        s_Color = s_Normal;

        HandleTools::popCamera();
        glPopMatrix();
    }

    return m_sMouse;
}


