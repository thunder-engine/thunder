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

layout(location = 6) in vec3 _view;
layout(location = 7) flat in vec4 _objectId;
layout(location = 8) flat in int _instanceOffset;
layout(location = 9) in mat4 _modelView;

layout(location = 0) out vec4 rgb;

void main(void) {
    rgb = _color;
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="true"/>
    </pass>
</shader>
