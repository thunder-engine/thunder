#version 450 core

#include "ShaderLayout.h"
#include "BRDF.h"

layout(location = 0) uniform mat4 t_model;
layout(location = 1) uniform mat4 t_view;

layout(location = 50) uniform sampler2D normalsMap;
layout(location = 51) uniform sampler2D diffuseMap;
layout(location = 52) uniform sampler2D paramsMap;
layout(location = 53) uniform sampler2D depthMap;
layout(location = 54) uniform sampler2D shadowMap;

layout(location = 55) uniform vec3 lightRight;
layout(location = 56) uniform vec3 lightUp;

layout(location = 0) in vec4 _vertex;

layout(location = 0) out vec4 rgb;

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
    vec2 proj   = ((_vertex.xyz / _vertex.w) * 0.5 + 0.5).xy;

    vec4 slice0 = texture(normalsMap,  proj);

    // Light model LIT
    if(slice0.w > 0.33) {
        float depth = texture(depthMap, proj).x;
        vec3 world  = getWorld(camera.screenToWorld, proj, depth);

        vec3 dir = light.position.xyz - world;
        float dist = length(dir);
        vec3 l = dir / dist;
        // Shadows step
        float factor = 1.0;
        if(light.shadows == 1.0) {
            int index = sampleCube(-l);
            vec4 offset = light.tiles[index];
            vec4 proj = light.matrix[index] * vec4(world, 1.0);
            vec3 coord = (proj.xyz / proj.w);
            factor = getShadow(shadowMap, (coord.xy * offset.zw) + offset.xy, coord.z - light.bias.x);
        }

        if(factor > 0.0) {
            // Material parameters
            vec4 slice1 = texture(paramsMap, proj);
            float rough = slice1.x;
            float metal = slice1.z;
            float spec  = slice1.w;

            vec4 slice2 = texture(diffuseMap, proj);
            vec3 albedo = slice2.xyz;

            // Vectors
            vec3 v = normalize(camera.position.xyz - world);
            vec3 n = normalize(slice0.xyz * 2.0 - 1.0);
            vec3 r = -reflect(v, n);

            float cosTheta = clamp(dot(l, n), -0.999, 0.999);

            vec3 lightPlaneNormal = light.direction;
            float lightWidth = light.params.y;
            float lightHeight = light.params.z;
            float cutoff = light.params.w;

            float halfWidth = lightWidth * 0.5;
            float halfHeight = lightHeight * 0.5;
            vec3 p0 = light.position.xyz - lightRight * halfWidth + lightUp * halfHeight;
            vec3 p1 = light.position.xyz - lightRight * halfWidth - lightUp * halfHeight;
            vec3 p2 = light.position.xyz + lightRight * halfWidth - lightUp * halfHeight;
            vec3 p3 = light.position.xyz + lightRight * halfWidth + lightUp * halfHeight;

            if(dot(world - light.position.xyz, lightPlaneNormal) > 0) {
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

            rgb = vec4(light.color.xyz * light.params.x * result * max(factor, 0.0), 1.0);
        } else {
            rgb = vec4(vec3(0.0), 1.0);
        }
    } else {
        rgb = vec4(vec3(0.0), 1.0);
    }
}
