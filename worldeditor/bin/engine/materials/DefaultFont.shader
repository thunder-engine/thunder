<shader version="11">
    <properties>
        <property type="vec4" name="mainColor"/>
        <property type="float" name="weight"/>
        <property binding="0" type="texture2d" name="mainTexture"/>
    </properties>
    <vertex><![CDATA[
#version 450 core

#pragma flags

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 0) out vec2 _uv;
layout(location = 1) out vec4 _color;
layout(location = 2) flat out vec4 _objectId;
layout(location = 3) flat out int _instanceOffset;

#include "ShaderLayout.h"

void main(void) {
#pragma offset

#pragma instance

#pragma objectId

    _uv = uv0;
    _color = color * mainColor;

    vec4 vertex = cameraWorldToScreen() * modelMatrix() * vec4(vertex, 1.0f);
#ifdef ORIGIN_TOP
    vertex.y = -vertex.y;
#endif
    gl_Position = vertex;
}
]]></vertex>
    <fragment><![CDATA[
#version 450 core

#pragma flags

layout(location = 0) in vec2 _uv;
layout(location = 1) in vec4 _color;
layout(location = 2) flat in vec4 _objectId;
layout(location = 3) flat in int _instanceOffset;

layout(location = 0) out vec4 color;

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D mainTexture;

const float softness = 0.0625f;

void main() {
#pragma instance

    float min = 1.0f - weight - softness;
    float max = 1.0f - weight + softness;

    float mask = smoothstep(min, max, texture(mainTexture, _uv).x);

    if(mask < 0.1f) {
        discard;
    }

#ifdef VISIBILITY_BUFFER
    color = _objectId;
    return;
#endif

    color = vec4(_color.xyz, mask * _color.w);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="true"/>
    </pass>
</shader>
