<shader version="11">
    <properties>
        <property type="vec4" name="mainColor"/>
        <property binding="0" type="texture2d" name="mainTexture" path=".embedded/invalid.png"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D mainTexture;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 7) flat in int _instanceOffset;

layout(location = 0) out vec4 color;

void main() {
#pragma instance

#ifdef VISIBILITY_BUFFER
    color = vec4(objectID);
    return;
#endif

    color = texture(mainTexture, _uv0.xy) * _color * mainColor;
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="true"/>
    </pass>
</shader>