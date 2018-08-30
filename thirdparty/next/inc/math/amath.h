#ifndef AMATH_H_HEADER_INCLUDED
#define AMATH_H_HEADER_INCLUDED

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define EPSILON 1e-6f
#define PI 3.14159265358979323846f
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

#define SQR(a) (a * a)
#define QUAD(a) (a * a * a)

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#define CLAMP(x, min, max)	((x < min) ? min : (x > max) ? max : x)
#define MIX(a, b, f)        (a * (1 - f) + b * f)
#define QMIX(a, b, c, f)    (a * SQR(1 - f) + b * 2 * f * (1 - f) + c * SQR(f))
#define CMIX(a, b, c, d, f) (a * QUAD(1 - f) + b * 3 * f * SQR(1 - f) + c * 3 * SQR(f) * (1 - f) + d * QUAD(f))

#define RANGE(min, max) (min + ((max - min) * ((areal)rand() / RAND_MAX)))

#include "math/vector2.h"
#include "math/vector3.h"
#include "math/vector4.h"

#include "math/matrix3.h"
#include "math/matrix4.h"

#include "math/quaternion.h"

#include "math/plane.h"

#include "math/aabb.h"
#include "math/obb.h"

#include "math/ray.h"

#include <vector>
typedef std::vector<Vector2>    Vector2Vector;
typedef std::vector<Vector3>    Vector3Vector;
typedef std::vector<Vector4>    Vector4Vector;

extern "C" {
    namespace amath {
        NEXT_LIBRARY_EXPORT int gausianKernel(areal radius, areal *samples, uint8_t maxSamples);

        NEXT_LIBRARY_EXPORT areal perlinNoise(areal x, areal y);
    }
}

#endif /* AMATH_H_HEADER_INCLUDED */
