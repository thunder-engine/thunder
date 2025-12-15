<shader version="11">
    <properties>
        <property binding="1" type="texture2d" name="mainTexture" target="true"/>
    </properties>
    <vertex><![CDATA[
#version 450 core

#pragma flags

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec2 _uv0;
layout(location = 1) flat out int _instanceOffset;

#include "ShaderLayout.h"

void main(void) {
    _instanceOffset = gl_InstanceIndex * 4;

#pragma instance

    vec4 pos = modelMatrix() * vec4(vertex, 1.0);
#ifdef ORIGIN_TOP
    pos.y = -pos.y;
#endif
    _uv0 = uv0;
    gl_Position = pos;
}
]]></vertex>
    <fragment><![CDATA[
#version 450 core

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM + 1) uniform sampler2D mainTexture;

layout(location = 0) in vec2 _uv0;
layout(location = 1) flat in int _instanceOffset;

layout(location = 0) out vec4 color;

void main() {
    color = vec4(texture(mainTexture, _uv0).xyz, 1.0);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
    </pass>
</shader>
