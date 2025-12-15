<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="direction" type="vec2" />
        <property name="size" type="vec2" />
        <property name="curve" type="vec4" count="8" />
        <property name="steps" type="int" />
        <property name="threshold" type="float" />
        <property name="rgbMap" binding="0" type="texture2d" target="true" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

const int _instanceOffset = 0;

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D rgbMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 rgb;

void main(void) {
#pragma instance

    vec4 sum = texture(rgbMap, _uv0) * curve[0].x;
    for(int i = 1; i < steps; i++) {
        vec2 offset = vec2(float(i)) * size * direction;
        int r = i / 4;
        int b = int(mod(4, i));
        sum += texture(rgbMap, _uv0 - offset) * curve[r][b];
        sum += texture(rgbMap, _uv0 + offset) * curve[r][b];
    }
    rgb = max(sum - threshold, 0.0);
}
]]></fragment>
    <pass type="PostProcess" twoSided="true" lightModel="Unlit" wireFrame="false">
        <blend op="Add" dst="One" src="One" />
    </pass>
</shader>
