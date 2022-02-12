#ifndef HANDLETOOLS_H
#define HANDLETOOLS_H

#include <stack>

#include <amath.h>

#include "handles.h"

using namespace std;

class ENGINE_EXPORT HandleTools {
public:
    HandleTools();

    static Vector3Vector pointsArc(const Quaternion &rotation, float size, float start, float angle, bool center = false);

    static float distanceToPoint(const Matrix4 &matrix, const Vector3 &position);

    static float distanceToPath(const Matrix4 &matrix, const Vector3Vector &points);

    static float distanceToMesh(const Matrix4 &matrix, const IndexVector &indices, const Vector3Vector &vertices);

    static void setViewProjection(const Matrix4 &view, const Matrix4 &projection);

protected:
    typedef stack<Matrix4> MatrixStack;

    static Matrix4 s_View;
    static Matrix4 s_Projection;
};

#endif // HANDLETOOLS_H
