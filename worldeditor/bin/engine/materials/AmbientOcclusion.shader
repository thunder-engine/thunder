<Shader>
    <Properties>
        <Property name="radius" type="float"/>
        <Property name="bias" type="float"/>
        <Property name="power" type="float"/>
        <Property name="samplesKernel" type="vec3" count="16"/>
        <Property name="depthMap" type="texture2D" binding="1" target="true"/>
        <Property name="normalsMap" type="texture2D" binding="2" target="true"/>
        <Property name="noiseMap" type="texture2D" binding="3" target="true"/>
    </Properties>
    <Fragment>
<![CDATA[
#version 450 core

#define MAX_SAMPLE_COUNT 16

#include "ShaderLayout.h"

layout(std140, binding = UNIFORM) uniform Uniforms {
    float radius;
    float bias;
    float power;
    vec3 samplesKernel[MAX_SAMPLE_COUNT];
} uni;

layout(binding = UNIFORM + 1) uniform sampler2D depthMap;
layout(binding = UNIFORM + 2) uniform sampler2D normalsMap;
layout(binding = UNIFORM + 3) uniform sampler2D noiseMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;

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
]]>
    </Fragment>
    <Pass type="PostProcess" blendMode="Opaque" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>
