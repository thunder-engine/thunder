<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property path=".embedded/invalid.png" name="mainTexture" binding="0" type="samplercube" />
    </properties>
    <fragment><![CDATA[	
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform samplerCube mainTexture;

layout(location = 0) in vec3 _vertex;

layout(location = 0) out vec4 rgb;

void main(void) {
    rgb = texture(mainTexture, _vertex);
}
]]></fragment>
    <vertex><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec3 _vertex;

void main(void) {
    _vertex = vertex;

    gl_Position = cameraWorldToScreen() * vec4(_vertex, 1.0);
}
]]></vertex>
    <pass type="Surface" twoSided="true" lightModel="Unlit" wireFrame="false">
        <depth comp="Less" write="true" test="true" />
    </pass>
</shader>
