<shader version="11">
    <properties>
        <property type="float" name="threshold"/>
        <property binding="0" type="texture2d" name="rgbMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = LOCAL) uniform Uniform {
    mat4 model;
    float threshold;
} uni;

layout(binding = UNIFORM) uniform sampler2D rgbMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 0) out vec4 color;

void main(void) {
    color = texture(rgbMap, _uv0 + g.cameraScreen.zw * vec2( 0.5, 0.5)) +
            texture(rgbMap, _uv0 + g.cameraScreen.zw * vec2(-0.5,-0.5)) +
            texture(rgbMap, _uv0 + g.cameraScreen.zw * vec2( 0.5,-0.5)) +
            texture(rgbMap, _uv0 + g.cameraScreen.zw * vec2(-0.5, 0.5));

    color = max(color * 0.25 - uni.threshold, 0.0);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true"/>
</shader>
