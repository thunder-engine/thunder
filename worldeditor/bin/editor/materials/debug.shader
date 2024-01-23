<shader>
    <properties>
        <property name="showAlpha" type="float"/>
        <property name="texture0" type="texture2D" binding="1" target="true"/>
    </properties>
    <vertex>
<![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"

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
    _vertex = l.model * vec4(vertex, 1.0);

    _color = color;
    _uv0 = uv0;
    gl_Position = _vertex;
}
]]>
    </vertex>
    <fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"
#include "Functions.h"

layout(binding = UNIFORM) uniform Uniforms {
    float showAlpha;
} uni;

layout(binding = UNIFORM + 1) uniform sampler2D texture0;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 color;

void main() {
    color = vec4(texture(texture0, _uv0).xyz, 1.0);
}
]]>
    </fragment>
    <pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</shader>
