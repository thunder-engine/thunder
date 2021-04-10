#define PI 3.141592654f

struct camera_data {
    float4              eye;
    float4              pos;
    float4              reserved;
    float4              up;
};

struct APlane {
    float3              normal;
    float3              point;
    float               d;
};

float4 rand(uint2 *seed) {
    const float4 invMaxInt = (float4)(1.0f/4294967296.0f, 1.0f/4294967296.0f, 1.0f/4294967296.0f, 0);
    uint x = (*seed).x * 17 + (*seed).y * 13123;
    (*seed).x  = (x << 13) ^ x;
    (*seed).y ^= (x << 7);

    uint4 tmp = (uint4)
    ( (x * (x * x * 15731 + 74323) + 871483),
      (x * (x * x * 13734 + 37828) + 234234),
      (x * (x * x * 11687 + 26461) + 137589), 0 );

    return convert_float4(tmp) * invMaxInt;
}

inline float fresnel(float VdotN, float eta) {
    float sqr_eta       = eta * eta;
    float etaCos        = eta * VdotN;
    float sqr_etaCos    = etaCos * etaCos;
    float one_minSqrEta = 1.0 - sqr_eta;
    float value         = etaCos - sqrt(one_minSqrEta + sqr_etaCos);
    value              *= value / one_minSqrEta;
    return min(1.0f, value * value);
}

float3 triangleWeights(const float3 *point, const float3 v1, const float3 v2, const float3 v3) {
    float3 ve1  = (float3)(v2 - v1);
    float3 ve2  = (float3)(v3 - v2);
    float3 ve3  = (float3)(v1 - v3);

    float3 p1   = (float3)(*point - v1);
    float3 p2   = (float3)(*point - v2);
    float3 p3   = (float3)(*point - v3);

    float s1    = length(cross(p2, ve2));
    float s2    = length(cross(p3, ve3));
    float s3    = length(cross(p1, ve1));

    float sum   = s1 + s2 + s3;

    return (float3)(s1 / sum, s2 / sum, s3 / sum);
}

struct APlane plane_set_points(const float3 v1, const float3 v2, const float3 v3) {
    struct APlane ret;
    float3 aux1, aux2;
    aux1    = v2 - v1;
    aux2    = v3 - v1;
    ret.normal  = cross(aux1, aux2);
    //ret.normal  = normalize(ret.normal);
    ret.point   = v1;
    ret.d       = dot(ret.normal, ret.point);
    return ret;
}
