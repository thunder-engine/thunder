<shader>
    <properties>
        <property name="direction" type="vec2"/>
        <property name="size" type="vec2"/>
        <property name="curve" type="vec4" count="8"/>
        <property name="steps" type="int"/>
        <property name="rgbMap" type="texture2D" binding="1" target="true"/>
    </properties>
    <fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(std140, binding = UNIFORM) uniform Uniforms {
    vec2 direction;
    vec2 size;
    vec4 curve[8];
    int steps;
} uni;

layout(binding = UNIFORM + 1) uniform sampler2D rgbMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 0) out vec4 rgb;

void main (void) {
    vec4 sum = texture(rgbMap, _uv0) * uni.curve[0].x;
    for(int i = 1; i < uni.steps; i++) {
        vec2 offset = vec2(float(i)) * uni.size * uni.direction;
        int r = i / 4;
        int b = int(mod(4, i));
        sum += texture(rgbMap, _uv0 - offset) * uni.curve[r][b];
        sum += texture(rgbMap, _uv0 + offset) * uni.curve[r][b];
    }
    rgb = sum;
}
]]>
    </fragment>
    <pass type="PostProcess" blendMode="Additive" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</shader>
