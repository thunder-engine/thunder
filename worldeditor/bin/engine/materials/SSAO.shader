<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="radius" type="float" />
        <property name="bias" type="float" />
        <property name="power" type="float" />
        <property name="samplesKernel" type="vec3" count="16" />
        <property name="depthMap" binding="0" type="texture2d" target="true" />
        <property name="normalsMap" binding="1" type="texture2d" target="true" />
        <property name="noiseMap" binding="2" type="texture2d" target="true" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#define MAX_SAMPLE_COUNT 16

#pragma flags

const int _instanceOffset = 0;

#include "ShaderLayout.h"
#include "Functions.h"

layout(binding = UNIFORM) uniform sampler2D depthMap;
layout(binding = UNIFORM + 1) uniform sampler2D normalsMap;
layout(binding = UNIFORM + 2) uniform sampler2D noiseMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) flat in mat4 _projectionInv;

layout(location = 0) out vec4 color;

void main(void) {
#pragma instance

    vec2 scale = vec2(screenSize().x / 4.0f, screenSize().y / 4.0f);

    float depth = texture(depthMap, _uv0).x;
    if(depth < 1.0f) {
        vec4 norm = texture(normalsMap, _uv0);
        if(norm.w > 0.0f) {
            vec3 viewPos = getWorld(_projectionInv, _uv0, depth);

            vec3 view = mat3(viewMatrix()) * (norm.xyz * 2.0f - 1.0f);

            vec3 normal = normalize(view);
            vec3 random = texture(noiseMap, _uv0 * scale).xyz;

            vec3 tangent = normalize(random - normal * dot(random, normal));
            vec3 bitangent = normalize(cross(normal, tangent));
            mat3 tbn = mat3(tangent, bitangent, normal);

            float ssao = 0.0f;
            for(int i = 0; i < MAX_SAMPLE_COUNT; i++) {
                vec3 samp = tbn * samplesKernel[i];
                samp = viewPos + samp * radius;

                vec4 offset = projectionMatrix() * vec4(samp, 1.0f);
                offset.xyz /= offset.w;
                offset.xyz  = offset.xyz * 0.5f + 0.5f;

                float sampleDepth = texture(depthMap, offset.xy).x;
                sampleDepth = getWorld(_projectionInv, offset.xy, sampleDepth).z;

                float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(viewPos.z - sampleDepth));
                ssao += step(samp.z + bias, sampleDepth) * rangeCheck;
            }
            color = vec4(vec3(1.0f - ssao / MAX_SAMPLE_COUNT), 1.0f);
            return;
        }
    }
    color = vec4(1.0f);
}
]]></fragment>
    <vertex><![CDATA[
#version 450 core

#pragma flags

const int _instanceOffset = 0;

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) flat out mat4 _projectionInv;

void main(void) {
    _vertex = vec4(vertex * 2.0, 1.0);
#ifdef ORIGIN_TOP
    _vertex.y = -_vertex.y;
#endif
    _uv0 = uv0;
    _projectionInv = projectionMatrixInv();
    gl_Position = _vertex;
}
]]></vertex>
    <pass type="PostProcess" twoSided="true" lightModel="Unlit" wireFrame="false" />
</shader>
