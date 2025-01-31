<shader version="11">
    <properties>
        <property type="vec4" name="color"/>
        <property type="float" name="width"/>
        <property binding="0" type="texture2d" name="outlineMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"
#include "Functions.h"

layout(binding = LOCAL) uniform Uniforms {
    mat4 model;
    vec4 color;
    float width;
} uni;

layout(binding = UNIFORM) uniform sampler2D outlineMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 rgb;

void main(void) {
    vec2 up = vec2(0.0, g.cameraScreen.w) * uni.width;
    vec2 right = vec2(g.cameraScreen.z, 0.0) * uni.width;

    float c = dot(texture(outlineMap, _uv0), vec4(1));
    float t = dot(texture(outlineMap, _uv0 - up), vec4(1));
    float l = dot(texture(outlineMap, _uv0 - right), vec4(1));
    float r = dot(texture(outlineMap, _uv0 + right), vec4(1));
    float b = dot(texture(outlineMap, _uv0 + up), vec4(1));

    vec2 n = vec2(-(t - b), (r - l));
    float v = (length( n ) > 0.1) ? 1.0 : 0.0;

    rgb = vec4(uni.color.xyz, v);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
    </pass>
</shader>
