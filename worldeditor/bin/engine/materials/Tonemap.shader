<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="rgbMap" binding="0" type="texture2d" target="true" />
        <property name="lutMap" binding="1" type="texture2d" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D rgbMap;
layout(binding = UNIFORM + 1) uniform sampler3D lutMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 rgb;

vec3 tonemapFilmic(vec3 x) {
    x *= 16.0;
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;

    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main (void) {
    //vec3 color = tonemapFilmic(texture(rgbMap, _uv0).xyz);
    vec3 color = texture(rgbMap, _uv0).xyz;

    rgb = vec4(texture(lutMap, color).xyz, 1.0f);
}

]]></fragment>
    <pass type="PostProcess" twoSided="true" lightModel="Unlit" wireFrame="false" />
</shader>
