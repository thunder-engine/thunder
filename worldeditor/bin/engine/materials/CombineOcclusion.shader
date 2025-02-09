<shader version="11">
    <properties>
        <property binding="0" type="texture2d" name="aoMap" target="true"/>
        <property binding="1" type="texture2d" name="normalsMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D aoMap;
layout(binding = UNIFORM + 1) uniform sampler2D normalsMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 color;

void main() {
    // Light model LIT
    if(texture(normalsMap, _uv0).w > 0.0) {
        color = vec4(0, 0, 0, 1.0 - texture(aoMap, _uv0).x);
    } else {
        color = vec4(0, 0, 0, 0);
    }
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
    </pass>
</shader>
