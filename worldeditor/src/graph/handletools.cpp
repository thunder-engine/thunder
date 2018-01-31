#include "handletools.h"

#include <components/camera.h>

#include <float.h>

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

void HandleTools::pushCamera(const Camera &camera, const Matrix4 &model) {
    Matrix4 v, p;

    m_sWidth    = camera.width();
    m_sHeight   = camera.height();
    camera.matrices(v, p);

    m_sProjection.push(p);
    m_sModelView.push(v * model);
}

void HandleTools::popCamera() {
    m_sProjection.pop();
    m_sModelView.pop();
}

Matrix4 HandleTools::modelView() {
    return m_sModelView.top();
}
