<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="scale" type="vec2" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 6) flat in vec4 _objectId;
layout(location = 7) flat in int _instanceOffset;

layout(location = 0) out vec4 rgb;

#include "ShaderLayout.h"

void main() {
#pragma instance

    float total = floor(_uv0.x * scale.x) + floor(_uv0.y * scale.y);

    if(mod(total, 2.0f) == 0.0f) {
        rgb = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    } else {
        rgb = vec4(0.25f, 0.25f, 0.25f, 1.0f);
    }
}
]]></fragment>
    <pass type="Surface" twoSided="true" lightModel="Unlit" wireFrame="false">
        <blend op="Add" dst="OneMinusSourceAlpha" src="SourceAlpha" />
        <depth comp="Less" write="false" test="true" />
    </pass>
</shader>
