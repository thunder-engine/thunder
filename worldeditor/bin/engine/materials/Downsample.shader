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

void main(void) {
    color = texture(rgbMap, _uv0 + screenSizeNorm() * vec2( 0.5f, 0.5f)) +
            texture(rgbMap, _uv0 + screenSizeNorm() * vec2(-0.5f,-0.5f)) +
            texture(rgbMap, _uv0 + screenSizeNorm() * vec2( 0.5f,-0.5f)) +
            texture(rgbMap, _uv0 + screenSizeNorm() * vec2(-0.5f, 0.5f));

    color *= 0.25f;
}
]]></fragment>
    <pass type="PostProcess" twoSided="true" lightModel="Unlit" wireFrame="false" />
</shader>
