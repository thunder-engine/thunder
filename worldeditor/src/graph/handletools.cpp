#include "handletools.h"

#include <qgl.h>

#include <components/camera.h>

HandleTools::MatrixStack HandleTools::m_sProjection;
HandleTools::MatrixStack HandleTools::m_sModelView;

int HandleTools::m_sWidth   = 0;
int HandleTools::m_sHeight  = 0;

template<typename T>
float distanceToSegment(const T &a, const T &b, const T &p) {
    T v = b - a;
    T w = p - a;

    float c1    = w.dot(v);
    if(c1 <= 0.0f) {
        return w.length();
    }

    float c2    = v.dot(v);
    if( c2 <= c1 ) {
        return (p - b).length();
    }

    T l = a + v * (c1 / c2);
    return (p - l).length();
}

HandleTools::HandleTools() {

}

float HandleTools::distanceToPoint(const Vector3 &position) {
    int viewport[4] = {0, 0, m_sWidth, m_sHeight};
    Vector3 ssp;
    Camera::project(position, m_sModelView.top(), m_sProjection.top(), viewport, ssp);
    ssp.y    = m_sHeight - ssp.y;

    return (Handles::m_sMouse - ssp).length();
}

float HandleTools::distanceToLine(const Vector3 &a, const Vector3 &b) {
    Vector3 ssa, ssb;
    int viewport[4] = {0, 0, m_sWidth, m_sHeight};
    Camera::project(a, m_sModelView.top(), m_sProjection.top(), viewport, ssa);
    Camera::project(b, m_sModelView.top(), m_sProjection.top(), viewport, ssb);
    ssa.y    = m_sHeight - ssa.y;
    ssb.y    = m_sHeight - ssb.y;

    return distanceToSegment(ssa, ssb, Handles::m_sMouse);
}

float HandleTools::distanceToPath(const Vector3List &points) {
    int viewport[4] = {0, 0, m_sWidth, m_sHeight};
    float result    = FLT_MAX;
    bool first      = true;
    Vector3 back;
    for(auto it : points) {
        Vector3 ss;
        Camera::project(it, m_sModelView.top(), m_sProjection.top(), viewport, ss);
        ss.y    = m_sHeight - ss.y;
        if(!first) {
            result  = std::min(distanceToSegment(back, ss, Handles::m_sMouse), result);
        } else {
            first   = false;
        }
        back    = ss;
    }
    return result;
}

void HandleTools::pushCamera(const Camera &camera) {
    Matrix4 mv, p;
    glGetFloatv(GL_MODELVIEW_MATRIX, mv.mat);
    glGetFloatv(GL_PROJECTION_MATRIX, p.mat);

    m_sWidth    = camera.width();
    m_sHeight   = camera.height();

    m_sProjection.push(p);
    m_sModelView.push(mv);
/*
    Matrix4 v, p;
    if(camera) {
        camera->matrices(v, p);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(p.mat);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(v.mat);
*/
}

void HandleTools::popCamera() {
    m_sProjection.pop();
    m_sModelView.pop();
}
