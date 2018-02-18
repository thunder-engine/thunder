#pragma version

#define MAX_SAMPLE_COUNT 16
#define RADIUS 1.0f
#define NORMAL_BIAS 0.0f;

#include ".embedded/Common.vert"

uniform sampler2D layer0;
uniform sampler2D depthMap;
uniform sampler2D noiseMap;

uniform vec3 samplesKernel[MAX_SAMPLE_COUNT];

layout(location = 1) in vec2 _uv0;
layout(location = 5) in mat4 _mvpi;
layout(location = 9) in mat4 _vp;

out vec4 rgb;

float depthAssessment_invsqrt(float nonLinearDepth) {
    return 1 / sqrt(1.0 - nonLinearDepth);
}

float occlusion(vec3 position, vec3 normal, vec3 dir, float radius, float depth) {
    vec3 p  = position + dir * radius;

    vec4 vp = _vp * vec4(p, 1.0f);
    vp.xy   = vec2(0.5f, 0.5f) + vec2(0.5f, -0.5f) * vp.xy / vp.w;
    vec3 projected  = vec3(vp.xy, vp.z / vp.w);

    float realDepth         = texture2D(depthMap, projected.xy).x;

    float assessProjected   = depthAssessment_invsqrt(projected.z);
    float assessReaded      = depthAssessment_invsqrt(realDepth);

    float differnce     = (assessReaded - assessProjected);

    float occlussion    = step(differnce, 0);
    float distanceCheck = min(1.0, radius / abs(depth - assessReaded));

    return occlussion * distanceCheck;
}

void main(void) {
    float depth = texture2D(depthMap,   _uv0 ).x;
    vec3 normal = texture2D(layer0,     _uv0 ).xyz;
    vec4 world  = getWorld(_mvpi,       _uv0, depth); // + normal * NORMAL_BIAS

    vec3 random     = normalize(texture2D(noiseMap, _uv0 /* * RNTextureSize */).xyz);
    float original  = depthAssessment_invsqrt(depth);

    float ssao  = 0;
    for(int i = 0; i < MAX_SAMPLE_COUNT; i++) {
        vec3 randomNormal   = reflect(samplesKernel[i], random);
        vec3 normalOriented = randomNormal * sign(dot(randomNormal, normal));

        ssao += occlusion(world.xyz / world.w, normal, normalOriented, RADIUS, original);
    }
    rgb = vec4(vec3(ssao / MAX_SAMPLE_COUNT), 1.0);
}
