#include "editor/handletools.h"

#include <components/camera.h>
#include <resources/mesh.h>

#include <float.h>

#define SIDES 180

Matrix4 HandleTools::s_View;
Matrix4 HandleTools::s_Projection;

HandleTools::HandleTools() {

}

Vector3Vector HandleTools::pointsArc(const Quaternion &rotation, float size, float start, float angle, bool center) {
    Vector3Vector result;
    int sides = abs(SIDES / 360.0f * angle);
    float theta = angle / float(sides - 1) * DEG2RAD;
    float tfactor = tanf(theta);
    float rfactor = cosf(theta);

    float x = size * cosf(start * DEG2RAD);
    float y = size * sinf(start * DEG2RAD);

    if(center) {
        result.push_back(Vector3());
    }

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
    Matrix4 mv = s_View * matrix;
    Vector3 ssp = Camera::project(position, mv, s_Projection);
    ssp.y = 1.0f - ssp.y;

    Vector2 ss(ssp.x, ssp.y);
    return (Handles::s_Mouse - ss).length();
}

float HandleTools::distanceToPath(const Matrix4 &matrix, const Vector3Vector &points) {
    Matrix4 mv = s_View * matrix;
    float result = FLT_MAX;
    bool first = true;
    Vector2 back;
    for(auto it : points) {
        Vector3 ssp = Camera::project(it, mv, s_Projection);
        ssp.y = 1.0f - ssp.y;
        Vector2 ss(ssp.x, ssp.y);
        if(!first) {
            result = std::min(distanceToSegment(back, ss, Handles::s_Mouse), result);
        } else {
            first = false;
        }
        back = ss;
    }
    return sqrtf(result);
}

float HandleTools::distanceToMesh(const Matrix4 &matrix, const Mesh *mesh) {
    Lod *lod = mesh->lod(0);
    if(!lod) {
        return -1.0f;
    }

    IndexVector &indices = lod->indices();
    Vector3Vector &vertices = lod->vertices();
    if(indices.empty()) {
        return distanceToPath(matrix, vertices);
    }
    Matrix4 mv = s_View * matrix;
    float result = FLT_MAX;
    if((vertices.size() % 2) == 0) {
        for(uint32_t i = 0; i < indices.size() - 1; i += 2) {
            Vector3 a = Camera::project(vertices[indices[i]], mv, s_Projection);
            a.y = 1.0f - a.y;
            Vector3 b = Camera::project(vertices[indices[i+1]], mv, s_Projection);
            b.y = 1.0f - b.y;
            Vector2 ssa(a.x, a.y);
            Vector2 ssb(b.x, b.y);
            result = std::min(distanceToSegment(ssa, ssb, Handles::s_Mouse), result);
        }
    }
    return sqrtf(result);
}

void HandleTools::setViewProjection(const Matrix4 &view, const Matrix4 &projection) {
    s_View = view;
    s_Projection = projection;
}
