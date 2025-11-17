<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="matrix" type="mat4" />
        <property name="tiles" type="vec4" />
        <property name="color" type="vec4" />
        <property name="params" type="vec4" />
        <property name="bias" type="vec4" />
        <property name="position" type="vec4" />
        <property name="direction" type="vec4" />
        <property name="shadows" type="float" />
        <property name="normalsMap" binding="0" type="texture2d" target="true" />
        <property name="diffuseMap" binding="1" type="texture2d" target="true" />
        <property name="paramsMap" binding="2" type="texture2d" target="true" />
        <property name="depthMap" binding="3" type="texture2d" target="true" />
        <property name="shadowMap" binding="4" type="texture2d" target="true" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

layout(location = 0) in vec4 _vertex;
layout(location = 1) flat in int _instanceOffset;
layout(location = 2) flat in mat4 _screenToWorld;

layout(location = 0) out vec4 rgb;

#include "ShaderLayout.h"
#include "Functions.h"
#include "BRDF.h"

layout(binding = UNIFORM) uniform sampler2D normalsMap;
layout(binding = UNIFORM + 1) uniform sampler2D diffuseMap;
layout(binding = UNIFORM + 2) uniform sampler2D paramsMap;
layout(binding = UNIFORM + 3) uniform sampler2D depthMap;
layout(binding = UNIFORM + 4) uniform sampler2D shadowMap;

void main (void) {
#pragma instance

    vec2 proj = ((_vertex.xyz / _vertex.w) * 0.5 + 0.5).xy;
#ifdef ORIGIN_TOP
    proj.y = 1.0 - proj.y;
#endif

    // params = x - brightness, y - radius/width, z - length/height, w - cutoff

    vec4 normalsSlice = texture(normalsMap,  proj);

    // Light model LIT
    if(normalsSlice.w > 0.0) {
        float depth = texture(depthMap, proj).x;
        vec3 world = getWorld(_screenToWorld, proj, depth);

        vec3 n = normalize(normalsSlice.xyz * 2.0 - 1.0);

        vec3 dir = position.xyz - world;
        vec3 l = normalize(dir);
        float dist = length(dir);

        float spot = dot(l, direction.xyz);
        float fall = 0.0;
        if(spot > params.w) {
            fall = 1.0 - (1.0 - spot) / (1.0 - params.w);
            fall = getAttenuation(dist, params.y) * params.x * fall;
        }

        vec4 paramsSlice = texture(paramsMap, proj);
        float rough = paramsSlice.x;
        float metal = paramsSlice.z;
        float spec = paramsSlice.w;

        vec4 diffuseSlice = texture(diffuseMap, proj);
        vec3 albedo = diffuseSlice.xyz;

        vec3 v = normalize(cameraPosition() - world);
        vec3 h = normalize(l + v);

        float cosTheta = clamp(dot(l, n), 0.0, 1.0);

        float shadow = 1.0;
        if(shadows > 0.0) {
            vec4 proj = matrix * vec4(world, 1.0);
            vec3 coord  = (proj.xyz / proj.w);
            shadow = getShadow(shadowMap, (coord.xy * tiles.zw) + tiles.xy, coord.z - bias.x);
        }

        vec3 refl = mix(vec3(spec), albedo, metal) * getCookTorrance(n, v, h, cosTheta, rough);
        vec3 result = albedo * (1.0 - metal) + refl;
        float diff = max(PI * getLambert(cosTheta, params.x) * fall * shadow, 0.0);

        rgb = vec4(color.xyz * result * diff, 1.0);
        return;
    }
    rgb = vec4(vec3(0.0), 1.0);
}
]]></fragment>
    <vertex><![CDATA[
#version 450 core

#pragma flags

layout(location = 0) in vec3 vertex;

layout(location = 0) out vec4 _vertex;
layout(location = 1) flat out int _instanceOffset;
layout(location = 2) flat out mat4 _screenToWorld;

#include "ShaderLayout.h"

void main(void) {
#pragma offset
#pragma instance

    _vertex = cameraWorldToScreen() * modelMatrix() * vec4(vertex, 1.0);
    _screenToWorld = cameraScreenToWorld();
    gl_Position = _vertex;
}
]]></vertex>
    <pass type="LightFunction" twoSided="false" lightModel="Unlit" wireFrame="false">
        <blend op="Add" dst="One" src="One" />
    </pass>
</shader>
