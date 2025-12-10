<shader version="11">
    <properties>
        <property type="float" name="focusDistance"/>
        <property type="float" name="focusScale"/>
        <property type="float" name="blurSize"/>
        <property type="float" name="skyDistance"/>
        <property binding="0" type="texture2d" name="highMap" target="true"/>
        <property binding="1" type="texture2d" name="lowMap" target="true"/>
        <property binding="2" type="texture2d" name="depthMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

const int _instanceOffset = 0;

#include "ShaderLayout.h"
#include "Functions.h"

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
const float goldenAngle = 2.39996323f;

float cocSize(float depth, float focusDistance, float focusScale, float blurSize) {
    float coc = clamp((depth - focusDistance) * focusScale, -1.0f, 1.0f);
    return abs(coc) * blurSize;
}

void main(void) {
#pragma instance

    color = texture(highMap, _uv0);

    float centerDepth = getLinearDepth(texture(depthMap, _uv0).x, nearClipPlane(), farClipPlane());
    if(centerDepth < skyDistance) {
        float centerSize = cocSize(centerDepth, focusDistance, focusScale, blurSize);

        float t = 1.0f;
        float radius = radScale;
        for(float ang = 0.0f; radius < blurSize; ang += goldenAngle) {
            vec2 tc = _uv0 + vec2(cos(ang), sin(ang)) * screenSizeNorm() * radius;

            vec3 sampleColor = texture(lowMap, tc).xyz;
            float sampleDepth = getLinearDepth(texture(depthMap, tc).x, nearClipPlane(), farClipPlane());

            float sampleSize = cocSize(sampleDepth, focusDistance, focusScale, blurSize);
            if(sampleDepth > centerDepth) {
                sampleSize = clamp(sampleSize, 0.0f, centerSize * 2.0f);
            }

            float m = smoothstep(radius - 0.5f, radius + 0.5f, sampleSize);
            color.xyz += mix(color.xyz / t, sampleColor, m);

            t += 1.0f;
            radius += radScale / radius;
        }
        color.xyz /= t;
    }
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true"/>
</shader>
