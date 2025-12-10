<shader version="11">
    <properties>
        <property type="mat4" name="matrix"/>
        <property type="vec4" name="tiles"/>
        <property type="vec4" name="color"/>
        <property type="vec4" name="params"/>
        <property type="vec4" name="bias"/>
        <property type="vec4" name="position"/>
        <property type="vec4" name="direction"/>
        <property type="float" name="shadows"/>
        <property binding="0" type="texture2d" name="normalsMap" target="true"/>
        <property binding="1" type="texture2d" name="diffuseMap" target="true"/>
        <property binding="2" type="texture2d" name="paramsMap" target="true"/>
        <property binding="3" type="texture2d" name="depthMap" target="true"/>
        <property binding="4" type="texture2d" name="shadowMap" target="true"/>
    </properties>
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

    vec4 slice0 = texture(normalsMap,  proj);

    // Light model LIT
    if(slice0.w > 0.0) {
        float depth = texture(depthMap, proj).x;
        vec3 world = getWorld(_screenToWorld, proj, depth);

        vec3 n = normalize(slice0.xyz * 2.0 - 1.0);

        vec3 dir = position.xyz - world;
        vec3 l = normalize(dir);
        float dist = length(dir);

        float spot = dot(l, direction.xyz);
        float fall = 0.0;
        if(spot > params.w) {
            fall = 1.0 - (1.0 - spot) / (1.0 - params.w);
            fall = getAttenuation(dist, params.y) * params.x * fall;
        }

        vec4 slice1 = texture(paramsMap, proj);
        float rough = slice1.x;
        float metal = slice1.z;
        float spec = slice1.w;

        vec4 slice2 = texture(diffuseMap, proj);
        vec3 albedo = slice2.xyz;

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
    <pass wireFrame="false" lightModel="Unlit" type="LightFunction" twoSided="false">
        <blend src="One" dst="One" op="Add"/>
    </pass>
</shader>
