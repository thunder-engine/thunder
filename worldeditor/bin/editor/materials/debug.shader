<shader version="11">
    <properties>
        <property binding="1" type="texture2d" name="texture0" target="true"/>
    </properties>
    <vertex><![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"
#include "VertexFactory.h"

#pragma uniforms

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec4 _color;

void main(void) {
    _vertex = getModelMatrix() * vec4(vertex, 1.0);

    _color = color;
    _uv0 = uv0;
    gl_Position = _vertex;
}
]]></vertex>
    <fragment><![CDATA[
#version 450 core

#include "ShaderLayout.h"
#include "Functions.h"

layout(binding = UNIFORM) uniform sampler2D texture0;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 color;

void main() {
    color = vec4(texture(texture0, _uv0).xyz, 1.0);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
    </pass>
</shader>
