<?xml version="1.0"?>
<shader version="14">
    <properties />
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 6) flat in vec4 _objectId;
layout(location = 7) flat in int _instanceOffset;

layout(location = 0) out vec4 rgb;

void main(void) {
    rgb = _color;
}
]]></fragment>
    <pass type="Surface" twoSided="true" lightModel="Unlit" wireFrame="false">
        <blend op="Add" dst="OneMinusSourceAlpha" src="SourceAlpha" />
        <depth comp="Less" write="false" test="true" />
    </pass>
</shader>
