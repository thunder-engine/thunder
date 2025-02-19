<shader version="11">
    <properties>
        <property type="float" name="focusDistance"/>
        <property type="float" name="focusScale"/>
        <property binding="0" type="texture2d" name="highMap" target="true"/>
        <property binding="1" type="texture2d" name="lowMap" target="true"/>
        <property binding="2" type="texture2d" name="depthMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"
#include "Functions.h"

layout(std140, binding = LOCAL) uniform InstanceData {
    mat4 model;
    float focusDistance;
    float focusScale;
} uni;

layout(binding = UNIFORM) uniform sampler2D highMap;
layout(binding = UNIFORM + 1) uniform sampler2D lowMap;
layout(binding = UNIFORM + 2) uniform sampler2D depthMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 color;

const vec2 circleOffsets[] = {
    vec2( 0.7071f, -0.7071f ),
    vec2(-0.7071f,  0.7071f ),
    vec2(-1.0000f,  0.0000f ),
    vec2( 1.0000f,  0.0000f ),
    vec2( 0.0000f,  0.0000f ),
    vec2(-0.7071f, -0.7071f ),
    vec2( 0.7071f,  0.7071f ),
    vec2( 0.0000f, -1.0000f ),
    vec2( 0.0000f,  1.0000f )
};

const float radScale = 0.5f;
const float blurSize = 20.0f;
const float goldenAngle = 2.39996323f;

float cocSize(float depth) {
    float coc = clamp((depth - uni.focusDistance) * uni.focusScale, -1.0f, 1.0f);
    return abs(coc) * blurSize;
}

void main(void) {
    color = texture(highMap, _uv0);

    float centerDepth = getLinearDepth(texture(depthMap, _uv0).x, g.cameraPosition.w, g.cameraTarget.w);
    float centerSize = cocSize(centerDepth);

    float t = 1.0f;
    float radius = radScale;
    for(float ang = 0.0f; radius < blurSize; ang += goldenAngle) {
        vec2 tc = _uv0 + vec2(cos(ang), sin(ang)) * g.cameraScreen.zw * radius;

        vec3 sampleColor = texture(lowMap, tc).xyz;
        float sampleDepth = getLinearDepth(texture(depthMap, tc).x, g.cameraPosition.w, g.cameraTarget.w);

        float sampleSize = cocSize(sampleDepth);
        if(sampleDepth > centerDepth) {
            sampleSize = clamp(sampleSize, 0.0, centerSize * 2.0f);
        }

        float m = smoothstep(radius - 0.5, radius + 0.5, sampleSize);
        color.xyz += mix(color.xyz / t, sampleColor, m);

        t += 1.0f;
        radius += radScale / radius;
    }
    color.xyz /= t;
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true"/>
</shader>
