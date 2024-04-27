<shader version="11">
    <properties>
        <property type="vec4" name="mainColor"/>
        <property binding="1" type="texture2d" name="mainTexture"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM + 1) uniform sampler2D mainTexture;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 7) flat in int _instanceOffset;

layout(location = 0) out vec4 color;

void main() {
#pragma instance

    color = texture(mainTexture, _uv0.xy) * _color * mainColor;
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="true"/>
    </pass>
</shader>
