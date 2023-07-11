<Shader>
    <Properties>
        <Property name="texture0" type="texture2D" binding="0" target="true"/>
    </Properties>
    <Vertex>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

#pragma flags

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;

layout(location = 0) out vec2 _uv0;

void main(void) {
    vec4 pos = vec4(vertex * 2.0, 1.0);
    #ifdef VULKAN
        pos.y = -pos.y;
    #endif
    _uv0 = uv0;
    gl_Position = pos;
}
]]>
    </Vertex>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D texture0;

layout(location = 0) in vec2 _uv0;

layout(location = 0) out vec4 color;

void main() {
    color = texture(texture0, _uv0);
}
]]>
    </Fragment>
    <Pass type="PostProcess" blendMode="Opaque" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>
