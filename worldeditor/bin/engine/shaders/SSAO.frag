#pragma version

#define MAX_SAMPLE_COUNT 16

#include "Common.vert"

layout(location = 2) uniform mat4 t_projection;

layout(location = 32) uniform sampler2D depthMap;
layout(location = 33) uniform sampler2D noiseMap;
layout(location = 34) uniform sampler2D normalsMap;
layout(location = 35) uniform sampler2D diffuseMap;

layout(location = 36) uniform float radius;
layout(location = 37) uniform float bias;
layout(location = 38) uniform float power;

layout(location = 39) uniform vec3 samplesKernel[MAX_SAMPLE_COUNT];

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 7) in vec3 _view;

layout(location = 0) out float color;

void main(void) {
    vec2 scale = vec2(camera.screen.z / 4.0, camera.screen.w / 4.0);

    mat4 projInv = inverse(camera.proj);

    float depth = texture(depthMap, _uv0).x;
    if(depth < 1.0) {
        vec4 world  = getWorld(projInv, _uv0, depth);
        world.xyz  /= world.w;

        vec3 normal = normalize(texture(normalsMap, _uv0).xyz * 2.0 - 1.0);
        vec3 random = texture(noiseMap, _uv0 * scale).xyz;

        vec3 tangent  = normalize(random - normal * dot(random, normal));
        vec3 binormal = cross(normal, tangent);
        mat3 tbn      = mat3(tangent, binormal, normal);

        float ssao = 0;
        for(int i = 0; i < MAX_SAMPLE_COUNT; i++) {
            vec3 samp = tbn * samplesKernel[i];
            samp = world.xyz + samp * radius;

            vec4 offset = vec4(samp, 1.0);
            offset      = camera.proj * offset;
            offset.xyz /= offset.w;
            offset.xyz  = offset.xyz * 0.5 + 0.5;

            float sampleDepth = texture(depthMap, offset.xy).x;
            vec4 sampWorld = getWorld(projInv, offset.xy, sampleDepth);
            sampWorld.xyz /= sampWorld.w;

            float rangeCheck = smoothstep(0.0, 1.0, radius / abs(world.z - sampWorld.z));
            ssao += step(samp.z + bias, sampWorld.z) * rangeCheck;
        }
        color = pow(1.0 - ssao / MAX_SAMPLE_COUNT, power);
    } else {
        color = 1.0;
    }
}
