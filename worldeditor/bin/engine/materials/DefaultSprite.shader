<shader version="11">
    <properties>
        <property type="vec4" name="mainColor"/>
        <property binding="0" type="texture2d" name="mainTexture" path=".embedded/invalid.png"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D mainTexture;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 6) in vec3 _view;
layout(location = 7) flat in vec4 _objectId;
layout(location = 8) flat in int _instanceOffset;
layout(location = 9) in mat4 _modelView;

layout(location = 0) out vec4 color;

void main() {
#pragma instance

    vec4 rgb = texture(mainTexture, _uv0.xy) * _color * mainColor;
    if(g.clip >= rgb.a) {
        discard;
    }

#ifdef VISIBILITY_BUFFER
    color = _objectId;
    return;
#endif

    color = rgb;
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="true"/>
    </pass>
</shader>
