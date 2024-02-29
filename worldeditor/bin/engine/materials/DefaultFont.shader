<shader>
    <properties>
        <property name="clipRect" type="vec4"/>
        <property name="weight" type="float"/>
        <property name="texture0" type="texture2D" binding="1" target="false"/>
    </properties>
    <vertex>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

#pragma flags

layout(binding = UNIFORM) uniform Uniforms {
    vec4 clipRect;

    float weight;
} uni;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 _uvMask;
layout(location = 1) out vec4 _color;

void main(void) {
    _uvMask = vec4(uv0, vertex.xy * 2.0 - uni.clipRect.xy - uni.clipRect.zw);
    _color = color;
    gl_Position = g.projection * ((g.view * l.model) * vec4(vertex, 1.0));
}
]]>
    </vertex>
    <fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform Uniforms {
    vec4 clipRect;

    float weight;
} uni;
layout(binding = UNIFORM + 1) uniform sampler2D texture0;

layout(location = 0) in vec4 _uvMask;
layout(location = 1) in vec4 _color;

layout(location = 0) out vec4 color;

void main() {
    float softness = 0.02f;

    float sdf = texture(texture0, _uvMask.xy).x;
    float mask = smoothstep(1.0f - uni.weight - softness, 1.0f - uni.weight + softness, sdf);

    color = vec4(l.color.xyz, mask);
}
]]>
    </fragment>
    <pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="true" depthWrite="false" twoSided="true"/>
</shader>
