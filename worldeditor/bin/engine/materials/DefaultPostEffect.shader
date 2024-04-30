<shader version="11">
    <properties>
        <property binding="0" type="texture2d" name="mainTexture" target="true"/>
    </properties>
	<vertex><![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"

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
]]></vertex>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D mainTexture;

layout(location = 0) in vec2 _uv0;

layout(location = 0) out vec4 color;

void main() {
    color = texture(mainTexture, _uv0);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true"/>
</shader>
