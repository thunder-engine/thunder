#include "editor/viewport/handletools.h"

#include <components/camera.h>
#include <resources/mesh.h>

#include <float.h>

float HandleTools::distanceToPoint(const Matrix4 &matrix, const Vector3 &point, const Vector2 &screen, bool world) {
    Vector3 p(matrix * point);
    if(world) {
        p = Camera::current()->project(p);
    }
    return (screen - p).length();
}

float HandleTools::distanceToPath(const Matrix4 &matrix, const Vector3Vector &points, const Vector2 &screen, bool world) {
    float result = FLT_MAX;
    bool first = true;
    Vector2 back;
    for(auto &it : points) {
        Vector3 p(matrix * it);
        if(world) {
            p = Camera::current()->project(p);
        }
        if(!first) {
            result = std::min(Mathf::distanceToSegment(back, Vector2(p), screen), result);
        } else {
            first = false;
        }
        back = p;
    }
    return sqrtf(result);
}

float HandleTools::distanceToMesh(const Matrix4 &matrix, const IndexVector &indices, const Vector3Vector &vertices, const Vector2 &screen, bool world) {
    if(indices.empty()) {
        return distanceToPath(matrix, vertices, screen);
    }
    float result = FLT_MAX;
    if((vertices.size() % 2) == 0) {
        for(uint32_t i = 0; i < indices.size() - 1; i += 2) {
            Vector3 a(matrix * vertices[indices[i]]);
            Vector3 b(matrix * vertices[indices[i+1]]);
            if(world) {
                a = Camera::current()->project(a);
                b = Camera::current()->project(b);
            }

            result = std::min(Mathf::distanceToSegment(Vector2(a), Vector2(b), screen), result);
        }
    }
    return sqrtf(result);
}

bool HandleTools::pointInRect(const Matrix4 &matrix, const Vector3 &tl, const Vector3 &br, const Vector2 &screen, bool world) {
    if(world) {
        Vector3 sstl = Camera::current()->project(matrix * tl);
        Vector3 ssbr = Camera::current()->project(matrix * br);

        return screen.x > sstl.x && screen.x < ssbr.x && screen.y < sstl.y && screen.y > ssbr.y;
    }
    return screen.x > tl.x && screen.x < br.x && screen.y < tl.y && screen.y > br.y;
}
