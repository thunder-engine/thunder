<shader version="11">
    <properties>
        <property binding="0" type="texture2d" name="depthMap" target="true"/>
        <property binding="1" type="texture2d" name="diffuseMap" target="true"/>
        <property binding="2" type="texture2d" name="normalsMap" target="true"/>
        <property binding="3" type="texture2d" name="paramsMap" target="true"/>
        <property binding="4" type="texture2d" name="aoMap" target="true"/>
        <property binding="5" type="texture2d" name="sslrMap" target="true"/>
        <property binding="6" type="samplercube" name="iblMap" target="true"/>
    </properties>
    <vertex><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;

layout(location = 0) out vec3 _vertex;

void main(void) {
    _vertex = vertex * 2.0f;
    gl_Position = vec4(_vertex, 1.0f);
}
]]></vertex>
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

layout(location = 0) out vec4 color;

#include "Functions.h"

void main(void) {
    vec2 proj = (_vertex.xyz * 0.5f + 0.5f).xy;

    proj.y = 1.0 - proj.y;

    float depth = texture(depthMap, proj).x;
    if(depth < 1.0) {
        vec4 normSamp = texture(normalsMap, proj);
        if(normSamp.a > 0.0f) {
            vec3 n = normSamp.xyz * 2.0f - 1.0f;

            vec3 world = getWorld(g.cameraScreenToWorld, proj, depth);
            vec3 v = normalize(world - g.cameraPosition.xyz);
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
    color = texture(iblMap, normalize(getWorld(g.cameraScreenToWorld, proj, 1.0f)));
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true">
        <blend src="One" dst="One" op="Add"/>
    </pass>
</shader>
