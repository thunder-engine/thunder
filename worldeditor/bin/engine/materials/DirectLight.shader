<shader version="11">
    <properties>
        <property count="4" type="mat4" name="matrix"/>
        <property count="4" type="vec4" name="tiles"/>
        <property type="vec4" name="color"/>
        <property type="vec4" name="params"/>
        <property type="vec4" name="direction"/>
        <property type="vec4" name="bias"/>
        <property type="vec4" name="planeDistance"/>
        <property type="float" name="shadows"/>
        <property binding="1" type="texture2d" name="normalsMap" target="true"/>
        <property binding="2" type="texture2d" name="diffuseMap" target="true"/>
        <property binding="3" type="texture2d" name="paramsMap" target="true"/>
        <property binding="4" type="texture2d" name="depthMap" target="true"/>
        <property binding="5" type="texture2d" name="shadowMap" target="true"/>
    </properties>
    <vertex><![CDATA[
#version 450 core

#pragma flags

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec4 _vertex;
layout(location = 1) flat out int _instanceOffset;
layout(location = 2) flat out mat4 _screenToWorld;

#include "ShaderLayout.h"

void main(void) {
    _instanceOffset = 0;

#pragma instance

    _vertex = vec4(vertex * 2.0, 1.0);
    _screenToWorld = cameraScreenToWorld();
    gl_Position = _vertex;
}
]]></vertex>
    <fragment><![CDATA[
#version 450 core

#pragma flags

layout(location = 0) in vec4 _vertex;
layout(location = 1) flat in int _instanceOffset;
layout(location = 2) flat in mat4 _screenToWorld;

#include "ShaderLayout.h"
#include "Functions.h"
#include "BRDF.h"

layout(binding = UNIFORM + 1) uniform sampler2D normalsMap;
layout(binding = UNIFORM + 2) uniform sampler2D diffuseMap;
layout(binding = UNIFORM + 3) uniform sampler2D paramsMap;
layout(binding = UNIFORM + 4) uniform sampler2D depthMap;
layout(binding = UNIFORM + 5) uniform sampler2D shadowMap;

layout(location = 0) out vec4 rgb;

void main(void) {
#pragma instance

    vec2 proj = ((_vertex.xyz / _vertex.w) * 0.5 + 0.5).xy;
#ifdef ORIGIN_TOP
    proj.y = 1.0 - proj.y;
#endif

    vec4 slice0 = texture(normalsMap, proj);

    // Light model LIT
    if(slice0.w > 0.0) {
        float depth = texture(depthMap, proj).x;
        vec3 world = getWorld(_screenToWorld, proj, depth);

        vec3 n = normalize(slice0.xyz * 2.0 - 1.0);

        vec4 params = texture(paramsMap, proj);
        float rough = params.x;
        float metal = params.z;
        float spec  = params.w;

        vec4 slice2 = texture(diffuseMap, proj);
        vec3 albedo = slice2.xyz;

        vec3 v = normalize(cameraPosition() - world);
        vec3 h = normalize(direction.xyz + v);

        float cosTheta = clamp(dot(direction.xyz, n), 0.0, 1.0);

        float shadow = 1.0;
        vec3 debugColor = vec3(1.0);
        if(shadows > 0.0) {
            int index = 3;
            float currentBias = 0.0;
            if(planeDistance.x > depth) {
                index = 0;
                debugColor = vec3(1, 0.5, 0.5);
                currentBias = bias.x;
            } else if(planeDistance.y > depth) {
                index = 1;
                debugColor = vec3(0.5, 1, 0.5);
                currentBias = bias.y;
            } else if(planeDistance.z > depth) {
                index = 2;
                debugColor = vec3(0.5, 0.5, 1);
                currentBias = bias.z;
            } else {
                debugColor = vec3(1, 0.5, 1);
                currentBias = bias.w;
            }

            vec4 offset = tiles[index];
            vec4 proj = matrix[index] * vec4(world, 1.0);
            vec3 coord = proj.xyz / proj.w;
            if(coord.x > 0.0 && coord.x < 1.0 && coord.y > 0.0 && coord.y < 1.0 && coord.z > 0.0 && coord.z < 1.0) {
                shadow = getShadow(shadowMap, (coord.xy * offset.zw) + offset.xy, coord.z - currentBias);
            }
        }

        vec3 refl = mix(vec3(spec), albedo, metal) * getCookTorrance(n, v, h, cosTheta, rough);
        vec3 result = albedo * (1.0 - metal) + refl;
        if(shadows > 1.0) {
            result *= debugColor;
        }

        float diff = getLambert(cosTheta, params.x) * shadow;

        rgb = vec4(color.xyz * result * diff, 1.0);
        return;
    }
    rgb = vec4(vec3(0.0), 1.0);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="LightFunction" twoSided="true">
        <blend src="One" dst="One" op="Add"/>
    </pass>
</shader>
