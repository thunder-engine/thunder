<shader version="11">
    <properties>
        <property binding="0" type="texture2d" name="depthMap" target="true"/>
        <property binding="1" type="texture2d" name="normalsMap" target="true"/>
        <property binding="2" type="texture2d" name="paramsMap" target="true"/>
        <property binding="3" type="texture2d" name="slrMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D depthMap;
layout(binding = UNIFORM + 1) uniform sampler2D normalsMap;
layout(binding = UNIFORM + 2) uniform sampler2D paramsMap;
layout(binding = UNIFORM + 3) uniform sampler2D slrMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 0) out vec3 color;

#include "Functions.h"

vec3 boxProjection(vec3 dir) {
    return dir;
}

void main(void) {
    float depth = texture(depthMap, _uv0).x;
    if(depth < 1.0) {
        vec3 origin = vec3(_uv0, depth);
        vec3 world = getWorld(g.cameraScreenToWorld, origin.xy, origin.z);

        vec3 v = normalize(world - g.cameraPosition.xyz);
        vec3 n = texture(normalsMap, _uv0).xyz * 2.0 - 1.0;
        vec3 refl = reflect(v, n);
        
        refl = boxProjection(refl);
        
        vec4 params = texture(paramsMap, _uv0);
        float rough = params.x;

        vec3 ibl = vec3(0.0);//textureLod(environmentMap, refl, rough * 10.0).xyz;

        vec4 sslr = texture(slrMap, _uv0);
        color = mix(ibl, sslr.xyz, sslr.w);
        return;
    }
    color = vec3(0.0);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true">
        <blend src="One" dst="One" op="Add"/>
    </pass>
</shader>
