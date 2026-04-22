<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="lod" type="float" />
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

layout(location = 0) out vec4 color;

void main(void) {
#pragma instance

    vec2 texelSize = screenSizeNorm();
    color = textureLod(rgbMap, _uv0 + texelSize * vec2( 0.5f, 0.5f), lod) +
            textureLod(rgbMap, _uv0 + texelSize * vec2(-0.5f,-0.5f), lod) +
            textureLod(rgbMap, _uv0 + texelSize * vec2( 0.5f,-0.5f), lod) +
            textureLod(rgbMap, _uv0 + texelSize * vec2(-0.5f, 0.5f), lod);

    color *= 0.25f;
}
]]></fragment>
    <pass type="PostProcess" twoSided="true" lightModel="Unlit" wireFrame="false" />
</shader>
