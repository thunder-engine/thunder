#include "handles.h"

#include <qgl.h>
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

float size  = 0.7f;
float sense =10.0f;

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

    glEnable(GL_BLEND);

    glClear(GL_DEPTH_BUFFER_BIT);
}

void Handles::endDraw() {
    glDisable(GL_BLEND);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void Handles::drawArrow(const Vector3 &position, const Quaternion &rotation, float size) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glMultMatrixf(Matrix4(rotation.toMatrix()).mat);

    float conesize  = size / 5.0f;

    s_System->setColor(s_Color);
    drawLine(Vector3(0.0f), Vector3(0, 0, size));
    s_System->setColor(s_Second);
    drawCone(Vector3(0, 0, size - conesize), Quaternion (), conesize);

    glPopMatrix();
}

void Handles::drawCone(const Vector3 &position, const Quaternion &rotation, float size) {
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    glMultMatrixf(Matrix4(rotation.toMatrix()).mat);

    s_System->setColor(s_Color);
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

void Handles::drawLine(const Vector3 &p1, const Vector3 &p2) {
    s_System->setColor(s_Color);

    s_System->drawPath({p1, p2});
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
    s_System->drawBillboard(position, size, texture);
}

Vector3 Handles::moveTool(const Vector3 &position, bool locked) {
    Vector3 result    = position;

    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);

    float scale = 0.0f;
    if(s_ActiveCamera) {
        scale   = (position - s_ActiveCamera->actor().position()).length() * cos(s_ActiveCamera->fov() * DEG2RAD) * 0.2f;
        glScalef(scale, scale, scale);

        float conesize  = size / 5.0f;
        float consize2  = conesize * 2.0f;

        Vector3 x( size, 0.0f, 0.0f);
        Vector3 y( 0.0f, size, 0.0f);
        Vector3 z( 0.0f, 0.0f, size);

        Vector3 xy( consize2, consize2, 0.0f);
        Vector3 yz( 0.0f, consize2, consize2);
        Vector3 xz( consize2, 0.0f, consize2);

        if(!locked) {
            HandleTools::pushCamera(*s_ActiveCamera);
            s_Axes  = AXIS_X | AXIS_Y | AXIS_Z;
            if(HandleTools::distanceToPoint(xy) <= sense * 2.0) {
                s_Axes  = AXIS_X | AXIS_Y;
            } else if(HandleTools::distanceToPoint(yz) <= sense * 2.0) {
                s_Axes  = AXIS_Y | AXIS_Z;
            } else if(HandleTools::distanceToPoint(xz) <= sense * 2.0) {
                s_Axes  = AXIS_X | AXIS_Z;
            } else if (HandleTools::distanceToLine(Vector3(), x) <= sense) {
                s_Axes  = AXIS_X;
            } else if (HandleTools::distanceToLine(Vector3(), y) <= sense) {
                s_Axes  = AXIS_Y;
            } else if(HandleTools::distanceToLine(Vector3(), z) <= sense) {
                s_Axes  = AXIS_Z;
            }
            HandleTools::popCamera();
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
        s_Color     = (s_Axes == (AXIS_X | AXIS_Z)) ? s_Selected : s_xColor;
        drawLine(Vector3(consize2, 0, 0), xz);
        s_Color     = (s_Axes == (AXIS_X | AXIS_Y)) ? s_Selected : s_xColor;
        drawLine(Vector3(consize2, 0, 0), xy);
        s_Color     = (s_Axes & AXIS_X) ? s_Selected : s_xColor;
        drawArrow(Vector3(conesize, 0, 0), Quaternion(Vector3(0, 1, 0), 90), size);

        s_Second    = s_yColor;
        s_Color = (s_Axes == (AXIS_X | AXIS_Y)) ? s_Selected : s_yColor;
        drawLine(Vector3(0, consize2, 0), xy);
        s_Color = (s_Axes == (AXIS_Y | AXIS_Z)) ? s_Selected : s_yColor;
        drawLine(Vector3(0, consize2, 0), yz);
        s_Color = (s_Axes & AXIS_Y) ? s_Selected : s_yColor;
        drawArrow(Vector3(0, conesize, 0), Quaternion(Vector3(1, 0, 0),-90), size);

        s_Second    = s_zColor;
        s_Color = (s_Axes == (AXIS_Y | AXIS_Z)) ? s_Selected : s_zColor;
        drawLine(Vector3(0, 0, consize2), yz);
        s_Color = (s_Axes == (AXIS_X | AXIS_Z)) ? s_Selected : s_zColor;
        drawLine(Vector3(0, 0, consize2), xz);
        s_Color = (s_Axes & AXIS_Z) ? s_Selected : s_zColor;
        drawArrow(Vector3(0, 0, conesize), Quaternion(Vector3(0, 0, 1), 90), size);

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
    }
    glPopMatrix();

    return result;
}

Vector3 Handles::rotationTool(const Vector3 &position, bool locked) {
    if(s_ActiveCamera) {
        float half      = 180.0f;
        float scale     = ((position - s_ActiveCamera->actor().position()).length() * cos(s_ActiveCamera->fov() / 2 * DEG2RAD) * 0.2f) * size;
        Vector3 normal  = position - s_ActiveCamera->actor().position();
        normal.normalize();

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        Quaternion q    = s_ActiveCamera->actor().rotation() * Quaternion(Vector3(90.0, 0, 0));

        Vector3List pb  = IRenderSystem::pointsArc(q, scale * 1.4f, 0, half * 2.0f);
        Vector3List px  = IRenderSystem::pointsArc(Quaternion(Vector3(0, 0, 1), 90),   scale,-RAD2DEG * atan2(normal.y, normal.z) + half, half);
        Vector3List py  = IRenderSystem::pointsArc(Quaternion(Vector3(0, 1, 0), half), scale,-RAD2DEG * atan2(normal.x, normal.z), half);
        Vector3List pz  = IRenderSystem::pointsArc(Quaternion(Vector3(1, 0, 0), 90),   scale, RAD2DEG * atan2(normal.x, normal.y), half);

        if(!locked) {
            HandleTools::pushCamera(*s_ActiveCamera);
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
        s_System->drawPath(pb);
        s_System->setColor(grey);
        s_System->drawPath(IRenderSystem::pointsArc(q, scale, 0, half * 2.0f));

        if(!locked || s_Axes == AXIS_X) {
            s_System->setColor((s_Axes == AXIS_X) ? s_Selected : s_xColor);
            s_System->drawPath(px);
        }

        if(!locked || s_Axes == AXIS_Y) {
            s_System->setColor((s_Axes == AXIS_Y) ? s_Selected : s_yColor);
            s_System->drawPath(py);
        }

        if(!locked || s_Axes == AXIS_Z) {
            s_System->setColor((s_Axes == AXIS_Z) ? s_Selected : s_zColor);
            s_System->drawPath(pz);
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
    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);

    float scale = 0.0f;
    if(s_ActiveCamera) {
        Vector3 normal    = position - s_ActiveCamera->actor().position();
        scale   = normal.length() * cos(s_ActiveCamera->fov() / 2 * DEG2RAD) * 0.2;
        glScalef(((normal.x < 0) ? 1 : -1) * scale,
                 ((normal.y > 0) ? 1 : -1) * scale,
                 ((normal.z < 0) ? 1 : -1) * scale);

        float half  = size * 0.5;
        float hh    = half * 0.5;
        float big   = half * 1.4;
        float hbig  = big  * 0.5;

        if(!locked) {
            HandleTools::pushCamera(*s_ActiveCamera);
            Vector3 x0  = Vector3(half, 0, 0);
            Vector3 y0  = Vector3(0,-half, 0);
            Vector3 z0  = Vector3(0, 0, half);

            Vector3 x1  = Vector3(size, 0, 0);
            Vector3 y1  = Vector3(0,-size, 0);
            Vector3 z1  = Vector3(0, 0, size);

            if(HandleTools::distanceToLine(x0, y0) <= sense) {
                s_Axes  = AXIS_X | AXIS_Y;
            } else if(HandleTools::distanceToLine(x0, z0) <= sense) {
                s_Axes  = AXIS_X | AXIS_Z;
            } else if(HandleTools::distanceToLine(y0, z0) <= sense) {
                s_Axes  = AXIS_Y | AXIS_Z;
            } else if(HandleTools::distanceToLine(x0, x1) <= sense) {
                s_Axes  = AXIS_X;
            } else if(HandleTools::distanceToLine(y0, y1) <= sense) {
                s_Axes  = AXIS_Y;
            } else if(HandleTools::distanceToLine(z0, z1) <= sense) {
                s_Axes  = AXIS_Z;
            }
            HandleTools::popCamera();
        }

        // X Axis
        glPushMatrix();
        glRotatef(90, 0, 1, 0);
        s_Color = (s_Axes == AXIS_X) ? s_Selected : s_xColor;
        drawLine(Vector3(), Vector3(0, 0, size));
        s_Color = (s_Axes & AXIS_X && s_Axes & AXIS_Y) ? s_Selected : s_xColor;
        drawLine(Vector3(0, 0, half), Vector3(0,-hh,   hh));
        drawLine(Vector3(0, 0, big),  Vector3(0,-hbig, hbig));
        s_Color = (s_Axes & AXIS_X && s_Axes & AXIS_Z) ? s_Selected : s_xColor;
        drawLine(Vector3(0, 0, half), Vector3(-hh,   0, hh));
        drawLine(Vector3(0, 0, big),  Vector3(-hbig, 0, hbig));
        glPopMatrix();
        // Y Axis
        glPushMatrix();
        glRotatef(90, 1, 0, 0);
        s_Color = (s_Axes == AXIS_Y) ? s_Selected : s_yColor;
        drawLine(Vector3(), Vector3(0, 0, size));
        s_Color = (s_Axes & AXIS_Y && s_Axes & AXIS_Z) ? s_Selected : s_yColor;
        drawLine(Vector3(0, 0, half), Vector3(0, hh,   hh));
        drawLine(Vector3(0, 0, big),  Vector3(0, hbig, hbig));
        s_Color = (s_Axes & AXIS_X && s_Axes & AXIS_Y) ? s_Selected : s_yColor;
        drawLine(Vector3(0, 0, half), Vector3(hh,   0,  hh));
        drawLine(Vector3(0, 0, big),  Vector3(hbig, 0, hbig));
        glPopMatrix();
        // Z Axis
        glPushMatrix();
        glRotatef(90, 0, 0, 1);
        s_Color = (s_Axes == AXIS_Z) ? s_Selected : s_zColor;
        drawLine(Vector3(), Vector3(0, 0, size));
        s_Color = (s_Axes & AXIS_X && s_Axes & AXIS_Z) ? s_Selected : s_zColor;
        drawLine(Vector3(0, 0, half), Vector3(0,-hh,   hh));
        drawLine(Vector3(0, 0, big),  Vector3(0,-hbig, hbig));
        s_Color = (s_Axes & AXIS_Y && s_Axes & AXIS_Z) ? s_Selected : s_zColor;
        drawLine(Vector3(0, 0, half), Vector3(-hh,   0, hh));
        drawLine(Vector3(0, 0, big),  Vector3(-hbig, 0, hbig));
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
    }
    glPopMatrix();

    return m_sMouse;
}


