<shader version="11">
    <properties>
        <property type="vec4" name="mainColor"/>
        <property type="float" name="scale"/>
        <property type="float" name="width"/>
        <property type="bool" name="ortho"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;
layout(location = 7) flat in int _instanceOffset;

layout(location = 0) out vec4 rgb;

const float subItems = 10.0;
const float cellSize = 10.0;

void main() {
#pragma instance

    vec3 pos = _vertex.xyz / _vertex.w;
    float fog = 1.0;
    if(!ortho) {
        fog = clamp(_color.w * mainColor.w * 100.0 * (1.0 - pos.z), 0.0, 1.0);
    }

    float cell = scale / subItems / cellSize;
    float w = cell * 0.2 * width;

    vec2 offset = _uv0 * subItems * scale;
    if(mod(offset.x, cell) < w || mod(offset.y, cell) < w) {
        rgb = vec4(1.0, 0.0, 0.0, fog);
        if(mod(offset.x, cell * subItems) < w || mod(offset.y, cell * subItems) < w) {
            rgb = vec4(_color.xyz * mainColor.xyz, fog);
        } else {
            rgb = vec4(_color.xyz * mainColor.xyz, fog * 0.5 * (1.0 - width));
        }
    } else {
        discard;
    }
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="true"/>
    </pass>
</shader>
