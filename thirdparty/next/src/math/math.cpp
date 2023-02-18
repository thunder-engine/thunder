#include "math/amath.h"

/*!
    \module Math

    \title Next Math Module

    \brief Contains classes for math operations.
*/

int Mathf::gausianKernel(areal radius, areal *samples, uint8_t maxSamples) {
    if(samples) {
        int32_t integerRadius   = maxSamples - 1;

        uint32_t count  = 0;
        areal sum = 0.0f;
        for(int32_t i = -integerRadius; i <= integerRadius; i += 2) {
            areal weight=  exp(-((i - 0) * (i - 0)) / (2.0f * radius)); // Normal Distribution

            samples[count] = weight;
            sum += weight;
            count++;
        }

        areal invSum = 1.0f / sum;
        for(uint32_t i = 0; i < count; ++i) {
            samples[i] *= invSum;
        }
        return count;
    }
    return 0;
}

inline areal noise2d(int x, int y) {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    int a = 15731; // primal
    int b = 789221; // primal
    int c = 1376312589; // primal
    int t = (n * (n * n * a + b) + c) & 0x7fffffff;
    return  1.0f - (areal)(t) / 1073741824.0f;
}

inline areal smoothed(int x, int y) {
    areal corners = (noise2d(x - 1, y - 1) + noise2d(x + 1, y - 1) +
                     noise2d(x - 1, y + 1) + noise2d(x + 1, y + 1)) / 16;
    areal sides   = (noise2d(x - 1, y) + noise2d(x + 1, y) +
                     noise2d(x, y - 1) + noise2d(x, y + 1)) /  8;
    areal center  =  noise2d(x, y) / 4;

    return corners + sides + center;
}

areal Mathf::perlinNoise(areal x, areal y) {
    int intx = int(x);
    areal fractionalx = x - intx;

    int inty = int(y);
    areal fractionaly = y - inty;

    areal v1 = smoothed(intx,     inty);
    areal v2 = smoothed(intx + 1, inty);
    areal v3 = smoothed(intx,     inty + 1);
    areal v4 = smoothed(intx + 1, inty + 1);

    areal t = (1.0f - (areal)cos(fractionalx * PI)) * 0.5f;
    areal i1 = MIX(v1, v2, t);
    areal i2 = MIX(v3, v4, t);

    return MIX(i1, i2, (1.0f - (areal)cos(fractionaly * PI)) * 0.5f);
}

Vector3Vector Mathf::pointsArc(const Quaternion &rotation, float size, float start, float angle, int steps, bool center) {
    Vector3Vector result;
    int sides = abs(steps / 360.0f * angle);
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
        float ty = x;

        x += tx * tfactor;
        y += ty * tfactor;

        x *= rfactor;
        y *= rfactor;
    }
    return result;
}

Vector3Vector Mathf::pointsCurve(const Vector3 &startPosition, const Vector3 &endPosition, const Vector3 &startTangent, const Vector3 &endTangent, int steps) {
    Vector3Vector points;
    points.resize(steps);
    for(int i = 0; i < steps; i++) {
        points[i] = CMIX(startPosition, startTangent, endTangent, endPosition, (float)i / float(steps-1));
    }
    return points;
}
