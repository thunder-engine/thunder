#ifndef HANDLETOOLS_H
#define HANDLETOOLS_H

#include <stack>

#include <amath.h>

#include "handles.h"

using namespace std;

class HandleTools {
public:
    HandleTools             ();

    static float            distanceToPoint     (const Vector3 &position);

    static float            distanceToPath      (const Vector3List &points);

    static void             pushCamera          (const Camera &camera, const Matrix4 &model);

    static void             popCamera           ();

    static Matrix4          modelView           ();

protected:
    typedef stack<Matrix4>  MatrixStack;

    static MatrixStack      m_sProjection;

    static MatrixStack      m_sModelView;

    static int              m_sWidth;
    static int              m_sHeight;
};

#endif // HANDLETOOLS_H
