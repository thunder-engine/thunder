<shader>
    <properties>
        <property name="clipRect" type="vec4"/>
        <property name="texture0" type="texture2D" binding="1" target="false"/>
    </properties>
    <vertex>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

#pragma flags

layout(binding = UNIFORM) uniform Uniforms {
    vec4 clipRect;
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
} uni;
layout(binding = UNIFORM + 1) uniform sampler2D texture0;

layout(location = 0) in vec4 _uvMask;
layout(location = 1) in vec4 _color;

layout(location = 0) out vec4 color;

void main() {
    float thickness = 0.5;
    float softness = 0.02;

    float oulineThickness = 0.6;
    float oulineSoftness = 0.02;

    float a = texture(texture0, _uvMask.xy).x;
    float outline = smoothstep(oulineThickness - oulineSoftness, oulineThickness + oulineSoftness, a);
    a = smoothstep(1.0 - thickness - softness, 1.0 - thickness + softness, a);

    vec2 m = clamp(uni.clipRect.zw - uni.clipRect.xy - abs(_uvMask.zw), 0.0, 1.0);
    //a *= 1.0 - step(m.x * m.y, 0.0);

    color = vec4(l.color.xyz, a);
}
]]>
    </fragment>
    <pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="true" depthWrite="false" twoSided="true"/>
</shader>
