#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "system.h"

#include <vector>
#include "math/amath.h"

class Texture;

typedef vector<Vector2>           Vector2List;
typedef vector<Vector3>           Vector3List;
typedef vector<Vector4>           Vector4List;

class NEXT_LIBRARY_EXPORT IRenderSystem : public ISystem {
public:
    IRenderSystem               (Engine *engine);

    virtual void                setColor                    (const Vector4 &color) = 0;

    virtual void                drawBillboard               (const Vector3 &position, const Vector2 &size, Texture &image) = 0;

    virtual void                drawPath                    (const Vector3List &points) = 0;

    static Vector3List          pointsArc                   (const Quaternion &rotation, float size, float start, float angle);
};

#endif // RENDERSYSTEM_H
