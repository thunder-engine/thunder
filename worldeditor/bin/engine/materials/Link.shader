<Shader>
    <Properties>
        <Property name="color0" type="vec4"/>
    </Properties>
    <Vertex>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

#pragma flags

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 _color;

void main(void) {
    _color = color;
    gl_Position = g.projection * ((g.view * l.model) * vec4(vertex, 1.0));
}
]]>
    </Vertex>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform Uniforms {
        vec4 color0;
} uni;

layout(location = 0) out vec4 rgb;

layout(location = 0) in vec4 _color;

void main(void) {   
    rgb = _color + uni.color0;
}
]]>
    </Fragment>
    <Pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="true" depthWrite="false" twoSided="true"/>
</Shader>
