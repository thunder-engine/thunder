<shader version="11">
    <properties>
        <property type="float" name="radius"/>
        <property type="float" name="bias"/>
        <property type="float" name="power"/>
        <property count="16" type="vec3" name="samplesKernel"/>
        <property binding="0" type="texture2d" name="depthMap" target="true"/>
        <property binding="1" type="texture2d" name="normalsMap" target="true"/>
        <property binding="2" type="texture2d" name="noiseMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#define MAX_SAMPLE_COUNT 16

#define NO_INSTANCE

#include "ShaderLayout.h"
#include "Functions.h"

layout(std140, binding = LOCAL) uniform Uniforms {
    mat4 model;
    float radius;
    float bias;
    float power;
    vec3 samplesKernel[MAX_SAMPLE_COUNT];
} uni;

layout(binding = UNIFORM) uniform sampler2D depthMap;
layout(binding = UNIFORM + 1) uniform sampler2D normalsMap;
layout(binding = UNIFORM + 2) uniform sampler2D noiseMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 0) out float color;

void main(void) {
    vec2 scale = vec2(g.cameraScreen.x / 4.0, g.cameraScreen.y / 4.0);

    float depth = texture(depthMap, _uv0).x;
    if(depth < 1.0) {
        vec3 world = getWorld(g.cameraProjectionInv, _uv0, depth);

        vec3 view = mat3(g.cameraView) * (texture(normalsMap, _uv0).xyz * 2.0 - 1.0);

        vec3 normal = normalize(view);
        vec3 random = texture(noiseMap, _uv0 * scale).xyz;

        vec3 tangent = normalize(random - normal * dot(random, normal));
        vec3 binormal = cross(normal, tangent);
        mat3 tbn = mat3(tangent, binormal, normal);

        float ssao = 0;
        for(int i = 0; i < MAX_SAMPLE_COUNT; i++) {
            vec3 samp = tbn * uni.samplesKernel[i];
            samp = world + samp * uni.radius;

            vec4 offset = vec4(samp, 1.0);
            offset = g.cameraProjection * offset;
            offset.xyz /= offset.w;
            offset.xyz  = offset.xyz * 0.5 + 0.5;

            float sampleDepth = texture(depthMap, offset.xy).x;
            vec3 sampWorld = getWorld(g.cameraProjectionInv, offset.xy, sampleDepth);

            float rangeCheck = smoothstep(0.0, 1.0, uni.radius / abs(world.z - sampWorld.z));
            ssao += step(samp.z + uni.bias, sampWorld.z) * rangeCheck;
        }
        color = pow(1.0 - ssao / MAX_SAMPLE_COUNT, uni.power);
    } else {
        color = 1.0;
    }
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true"/>
</shader>
