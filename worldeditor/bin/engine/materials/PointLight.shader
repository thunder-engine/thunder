<shader version="11">
    <properties>
        <property count="6" type="mat4" name="matrix"/>
        <property count="6" type="vec4" name="tiles"/>
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

int sampleCube(const vec3 v) {
    vec3 vAbs = abs(v);
    if(vAbs.z >= vAbs.x && vAbs.z >= vAbs.y) {
        return (v.z < 0.0) ? 5 : 4;
    } else if(vAbs.y >= vAbs.x) {
        return (v.y < 0.0) ? 3 : 2;
    }
    return (v.x < 0.0) ? 1 : 0;
}

void main (void) {
#pragma instance

    // params = x - brightness, y - radius/width, z - length/height, w - cutoff

    vec2 proj = ((_vertex.xyz / _vertex.w) * 0.5 + 0.5).xy;
#ifdef ORIGIN_TOP
    proj.y = 1.0 - proj.y;
#endif

    vec4 normalsSlice = texture(normalsMap,  proj);

    // Light model LIT
    if(normalsSlice.w > 0.0) {
        float depth = texture(depthMap, proj).x;
        vec3 world = getWorld(_screenToWorld, proj, depth);

        vec3 dir = position.xyz - world;
        float dist = length(dir);
        vec3 l = dir / dist;
        // Shadows step
        float factor = 1.0;
        if(shadows > 0.0) {
            int index = sampleCube(-l);
            vec4 offset = tiles[index];
            vec4 proj = matrix[index] * vec4(world, 1.0);
            vec3 coord = (proj.xyz / proj.w);
            factor = getShadow(shadowMap, (coord.xy * offset.zw) + offset.xy, coord.z - bias.x);
        }
        if(factor > 0.0) {
            // Material parameters
            vec4 paramsSlice = texture(paramsMap, proj);
            float rough = paramsSlice.x;
            float metal = paramsSlice.z;
            float spec  = paramsSlice.w;

            vec4 diffuseSlice = texture(diffuseMap, proj);
            vec3 albedo = diffuseSlice.xyz;

            // Vectors
            vec3 v = normalize(cameraPosition() - world);
            vec3 n = normalize(normalsSlice.xyz * 2.0 - 1.0);
            vec3 r = -reflect(v, n);

            float radius = params.y;
            float width = params.z;
            float cutoff = params.w;
            vec3 left = direction.xyz;

            float cosTheta = clamp(dot(l, n), 0.0, 1.0);

            vec3 spherePosition = position.xyz;
            vec3 sphereSpecularPosition = l;

            if(width > 0.0) {
                vec3 P0 = position.xyz - left * width * 0.5;
                vec3 P1 = position.xyz + left * width * 0.5;

                vec3 forward = normalize(closestPointOnLine(P0, P1, world) - world);
                vec3 up = cross(left, forward);

                float halfLength = 0.5 * width;

                vec3 p0 = position.xyz - left * halfLength + radius * up;
                vec3 p1 = position.xyz - left * halfLength - radius * up;
                vec3 p2 = position.xyz + left * halfLength - radius * up;
                vec3 p3 = position.xyz + left * halfLength + radius * up;

                spherePosition = closestPointOnSegment(P0, P1, world);

                // Specular part
                vec3 L0 = P0 - world;
                vec3 L1 = P1 - world;
                vec3 Ld = L1 - L0;

                float t = (dot(r, L0) * dot(r, Ld) - dot(L0, Ld)) / (dot(Ld, Ld) - sqr(dot(r, Ld)));

                sphereSpecularPosition = (L0 + clamp(t, 0.0, 1.0) * Ld);
            }

            if(radius > 0.0) {
                dir = spherePosition - world;
                dist = length(dir);
                l = dir / dist;

                cosTheta = clamp(dot(l, n), 0.0, 1.0);

                float sqrSphereDistance = dot(radius, radius);
                factor *= (PI * cosTheta * ((radius * radius) / sqrSphereDistance)) * getAttenuation(dist, cutoff - width * 0.5);

                // Specular part
                vec3 centerToRay = dot(sphereSpecularPosition, r) * r - sphereSpecularPosition;
                vec3 closestPoint = sphereSpecularPosition + centerToRay * clamp(radius / length(centerToRay), 0.0, 1.0);
                l = normalize(closestPoint);
                cosTheta = clamp(dot(l, n), 0.0, 1.0);
            } else {
                factor *= PI * cosTheta * getAttenuation(dist, cutoff);
            }
            // Combine step
            vec3 h = normalize(l + v);
            float refl = getCookTorrance(n, v, h, cosTheta, rough);

            vec3 result = albedo * (1.0 - metal) + (mix(vec3(spec), albedo, metal) * refl);

            rgb = vec4(color.xyz * params.x * result * max(factor, 0.0), 1.0);
            return;
        }
    }
    rgb = vec4(vec3(0.0), 1.0);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="LightFunction" twoSided="false">
        <blend src="One" dst="One" op="Add"/>
    </pass>
</shader>
