/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#ifndef AMATH_H
#define AMATH_H

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <random>

static std::random_device rd;
static std::mt19937 mt(rd());
static std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

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
#define QMIX(a, b, c, f)    (a * SQR((1 - f)) + b * 2 * f * (1 - f) + c * SQR(f))
#define CMIX(a, b, c, d, f) (a * QUAD((1 - f)) + b * 3 * f * SQR((1 - f)) + c * 3 * SQR(f) * (1 - f) + d * QUAD(f))

#define RANGE(min, max) (min + ((max - min) * (static_cast<areal>(dist(mt)) / static_cast<areal>(UINT32_MAX))))

#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

#include "matrix3.h"
#include "matrix4.h"

#include "quaternion.h"

#include "plane.h"

#include "aabb.h"
#include "obb.h"

#include "ray.h"

#include <vector>

typedef std::vector<Vector2> Vector2Vector;
typedef std::vector<Vector3> Vector3Vector;
typedef std::vector<Vector4> Vector4Vector;
typedef std::vector<uint32_t> IndexVector;

class NEXT_LIBRARY_EXPORT Mathf {
public:
    static int gausianKernel(areal radius, areal *samples, uint8_t maxSamples);
    static areal perlinNoise(areal x, areal y);

    template <typename T>
    static void hashCombine(uint32_t &seed, const T &v) {
        std::hash<T> hash;
        seed ^= hash(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    }

    inline static int hashString(const std::string &str) {
        int hash = 5381;
        for(auto it : str) {
            hash = ((hash << 5) + hash) + it;
        }

        return hash;
    }

    template<typename T>
    static float distanceToSegment(const T &a, const T &b, const T &p) {
        T v = b - a;
        T w = p - a;

        float c1 = w.dot(v);
        if(c1 <= 0.0f) {
            return w.sqrLength();
        }

        float c2 = v.dot(v);
        if( c2 <= c1 ) {
            return (p - b).sqrLength();
        }

        T l = a + v * (c1 / c2);
        return (p - l).sqrLength();
    }

    static Vector3Vector pointsArc(const Quaternion &rotation, float size, float start, float angle, int sides, bool center = false);

    static Vector3Vector pointsCurve(const Vector3 &startPosition, const Vector3 &endPosition, const Vector3 &startTangent, const Vector3 &endTangent, int steps);

};

#endif // AMATH_H
