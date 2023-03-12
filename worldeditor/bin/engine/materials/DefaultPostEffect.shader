<Shader>
    <Properties>
        <Property name="color" type="vec4"/>
        <Property name="texture0" type="texture2D" binding="1" target="true"/>
    </Properties>
    <Vertex>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

#pragma flags

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec4 _color;
layout(location = 3) out vec3 _n;
layout(location = 4) out vec3 _t;
layout(location = 5) out vec3 _b;

void main(void) {
    _vertex = vec4(vertex * 2.0, 1.0);
    #ifdef VULKAN
        _vertex.y = -_vertex.y;
    #endif
    _color = color;
    _uv0 = uv0;
    gl_Position = _vertex;
}
]]>
    </Vertex>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform Uniforms {
    vec4 color;
} uni;

layout(binding = UNIFORM + 1) uniform sampler2D texture0;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 0) out vec4 color;

void main() {
    color = texture(texture0, _uv0) * l.color * uni.color;
}
]]>
    </Fragment>
    <Pass type="PostProcess" blendMode="Translucent" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>
