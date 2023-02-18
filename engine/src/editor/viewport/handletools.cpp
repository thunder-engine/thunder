#include "editor/viewport/handletools.h"

#include <components/camera.h>
#include <resources/mesh.h>

#include <float.h>

Matrix4 HandleTools::s_View;
Matrix4 HandleTools::s_Projection;

float HandleTools::s_Sense = 0.02f;

float HandleTools::distanceToPoint(const Matrix4 &matrix, const Vector3 &point, const Vector2 &screen) {
    Matrix4 mv = s_View * matrix;
    Vector3 ssp = Camera::project(point, mv, s_Projection);

    return (screen - Vector2(ssp.x, ssp.y)).length();
}

float HandleTools::distanceToPath(const Matrix4 &matrix, const Vector3Vector &points, const Vector2 &screen) {
    Matrix4 mv = s_View * matrix;
    float result = FLT_MAX;
    bool first = true;
    Vector2 back;
    for(auto &it : points) {
        Vector3 ssp = Camera::project(it, mv, s_Projection);
        Vector2 ss(ssp.x, ssp.y);
        if(!first) {
            result = std::min(Mathf::distanceToSegment(back, ss, screen), result);
        } else {
            first = false;
        }
        back = ss;
    }
    return sqrtf(result);
}

float HandleTools::distanceToMesh(const Matrix4 &matrix, const IndexVector &indices, const Vector3Vector &vertices, const Vector2 &screen) {
    if(indices.empty()) {
        return distanceToPath(matrix, vertices, screen);
    }
    Matrix4 mv = s_View * matrix;
    float result = FLT_MAX;
    if((vertices.size() % 2) == 0) {
        for(uint32_t i = 0; i < indices.size() - 1; i += 2) {
            Vector3 a = Camera::project(vertices[indices[i]], mv, s_Projection);
            Vector3 b = Camera::project(vertices[indices[i+1]], mv, s_Projection);

            Vector2 ssa(a.x, a.y);
            Vector2 ssb(b.x, b.y);

            result = std::min(Mathf::distanceToSegment(ssa, ssb, screen), result);
        }
    }
    return sqrtf(result);
}
