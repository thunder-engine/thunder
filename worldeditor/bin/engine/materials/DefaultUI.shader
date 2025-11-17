<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="mainColor" type="vec4" />
        <property path=".embedded/invalid.png" name="mainTexture" binding="0" type="texture2d" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 6) flat in vec4 _objectId;
layout(location = 7) flat in int _instanceOffset;

layout(location = 0) out vec4 color;

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D mainTexture;

void main() {
#pragma instance

    vec4 rgb = texture(mainTexture, _uv0.xy);
    if(rgb.a < 0.1f) {
        discard;
    }

    color = rgb * _color * mainColor;
}
]]></fragment>
    <pass type="Surface" twoSided="true" lightModel="Unlit" wireFrame="false">
        <blend op="Add" dst="OneMinusSourceAlpha" src="SourceAlpha" />
    </pass>
</shader>
