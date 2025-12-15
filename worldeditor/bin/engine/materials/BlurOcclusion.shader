<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="aoMap" binding="0" type="texture2d" target="true" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D aoMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out float color;

const int blurRange = 2;

void main() {
    int n = 0;
    float result = 0.0;

    for(int x = -blurRange; x < blurRange; x++) {
        for(int y = -blurRange; y < blurRange; y++) {
            vec2 offset = vec2(float(x), float(y)) * screenSizeNorm();
            result += texture(aoMap, _uv0 + offset).r;
            n++;
        }
    }
    color = result / (float(n));
}
]]></fragment>
    <pass type="PostProcess" twoSided="true" lightModel="Unlit" wireFrame="false" />
</shader>
