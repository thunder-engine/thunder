#include "editor/viewport/handletools.h"

#include <components/camera.h>
#include <resources/mesh.h>

#include <float.h>

float HandleTools::distanceToPoint(const Matrix4 &matrix, const Vector3 &point, const Vector2 &screen) {
    Vector2 ssp = Camera::current()->project(matrix * point);

    return (screen - ssp).length();
}

float HandleTools::distanceToPath(const Matrix4 &matrix, const Vector3Vector &points, const Vector2 &screen) {
    float result = FLT_MAX;
    bool first = true;
    Vector2 back;
    for(auto &it : points) {
        Vector2 ssp = Camera::current()->project(matrix * it);
        if(!first) {
            result = std::min(Mathf::distanceToSegment(back, ssp, screen), result);
        } else {
            first = false;
        }
        back = ssp;
    }
    return sqrtf(result);
}

float HandleTools::distanceToMesh(const Matrix4 &matrix, const IndexVector &indices, const Vector3Vector &vertices, const Vector2 &screen) {
    if(indices.empty()) {
        return distanceToPath(matrix, vertices, screen);
    }
    float result = FLT_MAX;
    if((vertices.size() % 2) == 0) {
        for(uint32_t i = 0; i < indices.size() - 1; i += 2) {
            Vector2 a = Camera::current()->project(matrix * vertices[indices[i]]);
            Vector2 b = Camera::current()->project(matrix * vertices[indices[i+1]]);

            result = std::min(Mathf::distanceToSegment(a, b, screen), result);
        }
    }
    return sqrtf(result);
}
