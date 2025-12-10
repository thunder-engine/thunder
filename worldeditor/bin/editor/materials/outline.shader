<shader version="11">
    <properties>
        <property type="vec4" name="color"/>
        <property type="float" name="width"/>
        <property binding="0" type="texture2d" name="outlineMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

const int _instanceOffset = 0;

#include "ShaderLayout.h"
#include "Functions.h"

layout(binding = UNIFORM) uniform sampler2D outlineMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 rgb;

void main(void) {
#pragma instance

    vec2 up = vec2(0.0, screenSizeNorm().y) * width;
    vec2 right = vec2(screenSizeNorm().x, 0.0) * width;

    float c = dot(texture(outlineMap, _uv0), vec4(1));
    float t = dot(texture(outlineMap, _uv0 - up), vec4(1));
    float l = dot(texture(outlineMap, _uv0 - right), vec4(1));
    float r = dot(texture(outlineMap, _uv0 + right), vec4(1));
    float b = dot(texture(outlineMap, _uv0 + up), vec4(1));

    vec2 n = vec2(-(t - b), (r - l));
    float v = (length( n ) > 0.1) ? 1.0 : 0.0;

    rgb = vec4(color.xyz, v);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
    </pass>
</shader>
