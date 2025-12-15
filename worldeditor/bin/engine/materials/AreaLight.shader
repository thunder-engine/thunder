<shader version="11">
    <properties>
        <property count="6" type="mat4" name="matrix"/>
        <property count="6" type="vec4" name="tiles"/>
        <property type="vec4" name="color"/>
        <property type="vec4" name="params"/>
        <property type="vec4" name="bias"/>
        <property type="vec4" name="position"/>
        <property type="vec4" name="direction"/>
        <property type="vec4" name="right"/>
        <property type="vec4" name="up"/>
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
layout(binding = UNIFORM + 2) uniform sampler2D diffuseMap;
layout(binding = UNIFORM + 3) uniform sampler2D paramsMap;
layout(binding = UNIFORM + 4) uniform sampler2D depthMap;
layout(binding = UNIFORM + 5) uniform sampler2D shadowMap;

int sampleCube(const vec3 v) {
    vec3 vAbs = abs(v);
    if(vAbs.z >= vAbs.x && vAbs.z >= vAbs.y) {
        return (v.z < 0.0) ? 5 : 4;
    } else if(vAbs.y >= vAbs.x) {
        return (v.y < 0.0) ? 3 : 2;
    }
    return (v.x < 0.0) ? 1 : 0;
}

float intersectPlane(vec3 rayOrigin, vec3 rayDirection, vec3 planeOrigin, vec3 planeNormal) {
    return dot(planeNormal, (planeOrigin - rayOrigin) / dot(planeNormal, rayDirection));
}

float intersectTriangle(vec3 rayOrigin, vec3 rayDirection, vec3 A, vec3 B, vec3 C) {
    vec3 planeNormal = normalize(cross(B - A, C - B));
    float t = intersectPlane(rayOrigin, rayDirection, A, planeNormal);
    vec3 p = rayOrigin + rayDirection * t;

    vec3 N1 = normalize(cross(B - A, p - B));
    vec3 N2 = normalize(cross(C - B, p - C));
    vec3 N3 = normalize(cross(A - C, p - A));

    float d0 = dot(N1, N2);
    float d1 = dot(N2, N3);

    float threshold = 1.0f - 0.001f;
    return (d0 > threshold && d1 > threshold) ? 1.0f : 0.0f;
}

float intersectRectangle(vec3 rayOrigin, vec3 rayDirection, vec3 A, vec3 B, vec3 C, vec3 D) {
    return max(intersectTriangle(rayOrigin, rayDirection, A, B, C),
               intersectTriangle(rayOrigin, rayDirection, C, D, A));
}

float rectangleSolidAngle(vec3 worldPos, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
    vec3 v0 = p0 - worldPos;
    vec3 v1 = p1 - worldPos;
    vec3 v2 = p2 - worldPos;
    vec3 v3 = p3 - worldPos;

    vec3 n0 = normalize(cross(v0, v1));
    vec3 n1 = normalize(cross(v1, v2));
    vec3 n2 = normalize(cross(v2, v3));
    vec3 n3 = normalize(cross(v3, v0));

    float g0 = acos(dot(-n0, n1));
    float g1 = acos(dot(-n1, n2));
    float g2 = acos(dot(-n2, n3));
    float g3 = acos(dot(-n3, n0));

    return g0 + g1 + g2 + g3 - 2.0 * PI;
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

            float cosTheta = clamp(dot(l, n), -0.999, 0.999);

            vec3 lightPlaneNormal = direction.xyz;
            float lightWidth = params.y;
            float lightHeight = params.z;
            float cutoff = params.w;

            float halfWidth = lightWidth * 0.5;
            float halfHeight = lightHeight * 0.5;
            vec3 p0 = position.xyz - right.xyz * halfWidth + up.xyz * halfHeight;
            vec3 p1 = position.xyz - right.xyz * halfWidth - up.xyz * halfHeight;
            vec3 p2 = position.xyz + right.xyz * halfWidth - up.xyz * halfHeight;
            vec3 p3 = position.xyz + right.xyz * halfWidth + up.xyz * halfHeight;

            if(dot(world - position.xyz, lightPlaneNormal) > 0) {
                float solidAngle = rectangleSolidAngle(world, p0, p1, p2, p3);

                factor *= solidAngle * 0.25 * (clamp(dot(normalize(p0 - world), n), 0.0, 1.0) +
                                               clamp(dot(normalize(p1 - world), n), 0.0, 1.0) +
                                               clamp(dot(normalize(p2 - world), n), 0.0, 1.0) +
                                               clamp(dot(normalize(p3 - world), n), 0.0, 1.0)) * getAttenuation(dist, cutoff) * PI;
            } else {
                factor = 0.0;
            }

            if(factor > 0.0) {
                float traced = intersectRectangle(world, r, p0, p1, p2, p3);

                if(traced > 0.0) {
                    l = r;
                }
                cosTheta = dot(l, n);
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
