<shader version="11">
    <properties>
        <property binding="0" type="samplercube" name="mainTexture" path=".embedded/invalid.png"/>
    </properties>
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

    gl_Position = g.projection * (g.view * vec4(_vertex, 1.0));
}
]]></vertex>
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
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <depth comp="Less" write="true" test="true"/>
    </pass>
</shader>
