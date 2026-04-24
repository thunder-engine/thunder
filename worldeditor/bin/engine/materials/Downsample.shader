<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="rgbMap" binding="0" type="texture2d" target="true" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D rgbMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 color;

const int SAMPLES = 4;
const float offsets[4] = float[](-0.375, -0.125, 0.125, 0.375);
const float weights[4] = float[](0.1, 0.4, 0.4, 0.1);

void main(void) {
    vec2 texelSize = screenSizeNorm();

    vec4 result = vec4(0.0f);

    for(int i = 0; i < SAMPLES; i++) {
        for(int j = 0; j < SAMPLES; j++) {
            vec2 uv = _uv0 + vec2(offsets[i], offsets[j]) * texelSize;
            float weight = weights[i] * weights[j];
            result += texture(rgbMap, uv) * weight;
        }
    }
    color = result;
}
]]></fragment>
    <pass type="PostProcess" twoSided="true" lightModel="Unlit" wireFrame="false" />
</shader>
