#version 450 core

#define MAX_SAMPLE_COUNT 16

#include "ShaderLayout.h"

layout(location = 50) uniform sampler2D depthMap;
layout(location = 51) uniform sampler2D normalsMap;
layout(location = 52) uniform sampler2D noiseMap;

layout(location = 53) uniform float radius;
layout(location = 54) uniform float bias;
layout(location = 55) uniform float power;

layout(location = 56) uniform vec3 samplesKernel[MAX_SAMPLE_COUNT];

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 7) in vec3 _view;

layout(location = 0) out float color;

void main(void) {
    vec2 scale = vec2(camera.screen.z / 4.0, camera.screen.w / 4.0);

    float depth = texture(depthMap, _uv0).x;
    if(depth < 1.0) {
        vec3 world  = getWorld(camera.projectionInv, _uv0, depth);

        vec3 view = mat3(camera.view) * (texture(normalsMap, _uv0).xyz * 2.0 - 1.0);

        vec3 normal = normalize(view);
        vec3 random = texture(noiseMap, _uv0 * scale).xyz;

        vec3 tangent  = normalize(random - normal * dot(random, normal));
        vec3 binormal = cross(normal, tangent);
        mat3 tbn      = mat3(tangent, binormal, normal);

        float ssao = 0;
        for(int i = 0; i < MAX_SAMPLE_COUNT; i++) {
            vec3 samp = tbn * samplesKernel[i];
            samp = world + samp * radius;

            vec4 offset = vec4(samp, 1.0);
            offset      = camera.projection * offset;
            offset.xyz /= offset.w;
            offset.xyz  = offset.xyz * 0.5 + 0.5;

            float sampleDepth = texture(depthMap, offset.xy).x;
            vec3 sampWorld = getWorld(camera.projectionInv, offset.xy, sampleDepth);

            float rangeCheck = smoothstep(0.0, 1.0, radius / abs(world.z - sampWorld.z));
            ssao += step(samp.z + bias, sampWorld.z) * rangeCheck;
        }
        color = pow(1.0 - ssao / MAX_SAMPLE_COUNT, power);
    } else {
        color = 1.0;
    }
}
