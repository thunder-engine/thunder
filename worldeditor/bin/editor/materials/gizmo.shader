<shader version="11">
    <properties>
        <property binding="0" type="texture2d" name="depthMap" target="true"/>
    </properties>
    <vertex><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec4 _color;

void main(void) {
    _vertex = cameraWorldToScreen() * vec4(vertex, 1.0);
#ifdef ORIGIN_TOP
    _vertex.y = -_vertex.y;
#endif
    _color = color;
    gl_Position = _vertex;
}
]]></vertex>
    <fragment><![CDATA[	
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D depthMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec4 _color;

layout(location = 0) out vec4 rgb;

#include "Functions.h"

void main(void) {
    vec2 proj = ((_vertex.xyz / _vertex.w) * 0.5 + 0.5).xy;
#ifdef ORIGIN_TOP
    proj.y = 1.0 - proj.y;
#endif
    float depth = getLinearDepth(texture(depthMap, proj).x, nearClipPlane(), farClipPlane());
    rgb = (depth >= _vertex.z) ? _color : vec4(_color.xyz, _color.w * 0.25);
}
]]></fragment>
    <pass wireFrame="true" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
    </pass>
</shader>
