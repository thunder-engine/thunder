<shader version="11">
    <properties>
        <property binding="0" type="texture2d" name="rgbMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D rgbMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 color;

void main(void) {
    color = texture(rgbMap, _uv0 + g.cameraScreen.zw * vec2( 0.5f, 0.5f)) +
            texture(rgbMap, _uv0 + g.cameraScreen.zw * vec2(-0.5f,-0.5f)) +
            texture(rgbMap, _uv0 + g.cameraScreen.zw * vec2( 0.5f,-0.5f)) +
            texture(rgbMap, _uv0 + g.cameraScreen.zw * vec2(-0.5f, 0.5f));

    color *= 0.25f;
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true"/>
</shader>
