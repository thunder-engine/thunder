#include "handletools.h"

#include <components/camera.h>
#include <resources/mesh.h>

#include <float.h>

#define SIDES 36

Matrix4 HandleTools::s_View;
Matrix4 HandleTools::s_Projection;

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

Vector3Vector HandleTools::pointsArc(const Quaternion &rotation, float size, float start, float angle) {
    Vector3Vector result;
    int sides       = SIDES / 360.0 * angle;
    float theta     = angle / float(sides - 1) * DEG2RAD;
    float tfactor   = tanf(theta);
    float rfactor   = cosf(theta);

    float x = size * cosf(start * DEG2RAD);
    float y = size * sinf(start * DEG2RAD);

    for(int i = 0; i < sides; i++) {
        result.push_back(rotation * Vector3(x, 0, y));

        float tx = -y;
        float ty =  x;

        x += tx * tfactor;
        y += ty * tfactor;

        x *= rfactor;
        y *= rfactor;
    }
    return result;
}

float HandleTools::distanceToPoint(const Matrix4 &matrix, const Vector3 &position) {
    Matrix4 mv  = s_View * matrix;
    int viewport[4] = {0, 0, m_sWidth, m_sHeight};
    Vector3 ssp;
    Camera::project(position, mv, s_Projection, viewport, ssp);
    ssp.y    = m_sHeight - ssp.y;

    return (Handles::m_sMouse - ssp).length();
}

float HandleTools::distanceToPath(const Matrix4 &matrix, const Vector3Vector &points) {
    Matrix4 mv  = s_View * matrix;
    int viewport[4] = {0, 0, m_sWidth, m_sHeight};
    float result    = FLT_MAX;
    bool first      = true;
    Vector3 back;
    for(auto it : points) {
        Vector3 ss;
        Camera::project(it, mv, s_Projection, viewport, ss);
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

float HandleTools::distanceToMesh(const Matrix4 &matrix, const Mesh *mesh, uint32_t surface) {
    Mesh::IndexVector indices   = mesh->indices(surface, 0);
    Vector3Vector vertices      = mesh->vertices(surface, 0);
    if(indices.empty()) {
        return distanceToPath(matrix, vertices);
    }

    Matrix4 mv  = s_View * matrix;

    float result    = FLT_MAX;
    if((vertices.size() % 2) == 0) {
        int viewport[4] = {0, 0, m_sWidth, m_sHeight};
        for(uint32_t i = 0; i < indices.size() - 1; i += 2) {
            Vector3 a;
            Camera::project(vertices[indices[i]], mv, s_Projection, viewport, a);
            a.y     = m_sHeight - a.y;
            Vector3 b;
            Camera::project(vertices[indices[i+1]], mv, s_Projection, viewport, b);
            b.y     = m_sHeight - b.y;
            result  = std::min(distanceToSegment(a, b, Handles::m_sMouse), result);
        }
    }

    return result;
}

void HandleTools::setCamera(const Camera &camera) {
    camera.matrices(s_View, s_Projection);

    m_sWidth    = camera.width();
    m_sHeight   = camera.height();
}
