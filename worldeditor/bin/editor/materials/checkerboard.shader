<shader version="11">
    <properties/>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 0) out vec4 rgb;

void main() {
    float total = floor(_uv0.x * 20.0f) + floor(_uv0.y * 20.0f);

    if(mod(total, 2.0f) == 0.0f) {
        rgb = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    } else {
        rgb = vec4(0.25f, 0.25f, 0.25f, 1.0f);
    }
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="true"/>
    </pass>
</shader>
