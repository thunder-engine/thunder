#ifndef HANDLETOOLS_H
#define HANDLETOOLS_H

#include <amath.h>

#include <engine.h>

class Camera;

class ENGINE_EXPORT HandleTools {
public:
    static float distanceToPoint(const Matrix4 &matrix, const Vector3 &point, const Vector2 &screen);

    static float distanceToPath(const Matrix4 &matrix, const Vector3Vector &points, const Vector2 &screen);

    static float distanceToMesh(const Matrix4 &matrix, const IndexVector &indices, const Vector3Vector &vertices, const Vector2 &screen);

};

#endif // HANDLETOOLS_H
