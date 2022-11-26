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

inline areal noise2d(areal x, areal y) {
    int n = int(x) + int(y) * 57;
    n = (n << 13) ^ n;
    return ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

inline areal smoothed(areal x, areal y) {
    areal corners = (noise2d(x - 1, y - 1) + noise2d(x + 1, y - 1) + noise2d(x - 1, y + 1) + noise2d(x + 1, y + 1) ) / 16;
    areal sides   = (noise2d(x - 1, y) + noise2d(x + 1, y) + noise2d(x, y - 1) + noise2d(x, y + 1) ) /  8;
    areal center  =  noise2d(x, y) / 4;

    return corners + sides + center;
}

areal Mathf::perlinNoise(areal x, areal y) {
    areal intx = areal(int(x));
    areal fractionalx = x - intx;

    areal inty = areal(int(y));
    areal fractionaly = y - inty;

    areal v1 = smoothed(intx,     inty);
    areal v2 = smoothed(intx + 1, inty);
    areal v3 = smoothed(intx,     inty + 1);
    areal v4 = smoothed(intx + 1, inty + 1);

    areal i1 = MIX(v1, v2, fractionalx);
    areal i2 = MIX(v3, v4, fractionalx);

    return MIX(i1, i2, (1.0f - (areal)cos(fractionaly * PI)) * 0.5f);
}


