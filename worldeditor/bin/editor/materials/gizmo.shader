<shader version="11">
    <properties>
        <property binding="0" type="texture2d" name="depthMap" target="true"/>
    </properties>
    <fragment><![CDATA[	
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D depthMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 6) in vec3 _view;
layout(location = 7) flat in vec4 _objectId;
layout(location = 8) flat in int _instanceOffset;
layout(location = 9) in mat4 _modelView;

layout(location = 0) out vec4 rgb;

#include "Functions.h"

void main(void) {
    vec2 proj = ((_vertex.xyz / _vertex.w) * 0.5 + 0.5).xy;

    float depth = getLinearDepth(texture(depthMap, proj).x, g.cameraPosition.w, g.cameraTarget.w);
    rgb = (depth >= _vertex.z) ? _color : vec4(_color.xyz, _color.w * 0.25);
}
]]></fragment>
    <pass wireFrame="true" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
    </pass>
</shader>
