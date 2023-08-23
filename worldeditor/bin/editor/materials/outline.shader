<shader>
    <properties>
        <property name="color" type="vec4"/>
        <property name="width" type="float"/>
        <property name="rgbMap" type="texture2D" binding="1" target="true"/>
        <property name="outlineMap" type="texture2D" binding="2" target="true"/>
    </properties>
    <fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"
#include "Functions.h"

layout(binding = UNIFORM) uniform Uniforms {
    vec4 color;
    float width;
} uni;

layout(binding = UNIFORM + 1) uniform sampler2D rgbMap;
layout(binding = UNIFORM + 2) uniform sampler2D outlineMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 0) out vec4 rgb;

void main(void) {
    vec2 up = vec2(0.0, g.cameraScreen.w) * uni.width;
    vec2 right = vec2(g.cameraScreen.z, 0.0) * uni.width;

    float c = dot(texture(outlineMap, _uv0), vec4(1));
    float t = dot(texture(outlineMap, _uv0 - up), vec4(1));
    float l = dot(texture(outlineMap, _uv0 - right), vec4(1));
    float r = dot(texture(outlineMap, _uv0 + right), vec4(1));
    float b = dot(texture(outlineMap, _uv0 + up), vec4(1));

    vec2 n = vec2(-(t - b), (r - l));
    float v = (length( n ) > 0.1) ? 1.0 : 0.0;

    rgb = vec4(uni.color.xyz, v);
}
]]>
    </fragment>
    <pass type="PostProcess" blendMode="Translucent" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</shader>
