<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="depthMap" binding="0" type="texture2d" target="true" />
        <property name="diffuseMap" binding="1" type="texture2d" target="true" />
        <property name="normalsMap" binding="2" type="texture2d" target="true" />
        <property name="paramsMap" binding="3" type="texture2d" target="true" />
        <property name="aoMap" binding="4" type="texture2d" target="true" />
        <property name="sslrMap" binding="5" type="texture2d" target="true" />
        <property name="iblMap" binding="6" type="samplercube" target="true" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D depthMap;
layout(binding = UNIFORM + 1) uniform sampler2D diffuseMap;
layout(binding = UNIFORM + 2) uniform sampler2D normalsMap;
layout(binding = UNIFORM + 3) uniform sampler2D paramsMap;
layout(binding = UNIFORM + 4) uniform sampler2D aoMap;
layout(binding = UNIFORM + 5) uniform sampler2D sslrMap;
layout(binding = UNIFORM + 6) uniform samplerCube iblMap;

layout(location = 0) in vec3 _vertex;
layout(location = 1) flat in mat4 _screenToWorld;

layout(location = 0) out vec4 color;

#include "Functions.h"

void main(void) {
    vec2 proj = (_vertex.xyz * 0.5f + 0.5f).xy;

#ifdef ORIGIN_TOP
    proj.y = 1.0 - proj.y;
#endif

    float depth = texture(depthMap, proj).x;
    if(depth < 1.0) {
        vec4 normSamp = texture(normalsMap, proj);
        if(normSamp.a > 0.0f) {
            vec3 n = normSamp.xyz * 2.0f - 1.0f;

            vec3 world = getWorld(_screenToWorld, proj, depth);
            vec3 v = normalize(world - cameraPosition());
            vec3 refl = reflect(v, n);

            float rough = texture(paramsMap, proj).x;

            vec4 sslr = texture(sslrMap, proj);
            vec3 ibl = texture(iblMap, mix(refl, n, rough)).xyz;
            float occlusion = texture(aoMap, proj).x;
            color = vec4(mix(ibl, sslr.xyz, sslr.w) * texture(diffuseMap, proj).xyz * occlusion, 1.0);
        } else { // material is emissive no indirect
            color = vec4(0.0f);
        }
        return;
    }
    color = texture(iblMap, normalize(getWorld(_screenToWorld, proj, 1.0f)));
}
]]></fragment>
    <vertex><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;

layout(location = 0) out vec3 _vertex;
layout(location = 1) flat out mat4 _screenToWorld;

void main(void) {
    _vertex = vertex * 2.0f;
    _screenToWorld = cameraScreenToWorld();
    gl_Position = vec4(_vertex, 1.0f);
}
]]></vertex>
    <pass type="PostProcess" twoSided="true" lightModel="Unlit" wireFrame="false">
        <blend op="Add" dst="One" src="One" />
    </pass>
</shader>
