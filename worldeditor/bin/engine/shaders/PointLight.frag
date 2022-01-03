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

            float radius = light.params.y;
            float width = light.params.z;
            float cutoff = light.params.w;
            vec3 left = light.direction;

            float cosTheta = clamp(dot(l, n), 0.0, 1.0);

            vec3 spherePosition = light.position.xyz;
            vec3 sphereSpecularPosition = l;

            if(width > 0.0) {
                vec3 P0 = light.position.xyz - left * width * 0.5;
                vec3 P1 = light.position.xyz + left * width * 0.5;

                vec3 forward = normalize(closestPointOnLine(P0, P1, world) - world);
                vec3 up = cross(left, forward);

                float halfLength = 0.5 * width;

                vec3 p0 = light.position.xyz - left * halfLength + radius * up;
                vec3 p1 = light.position.xyz - left * halfLength - radius * up;
                vec3 p2 = light.position.xyz + left * halfLength - radius * up;
                vec3 p3 = light.position.xyz + left * halfLength + radius * up;

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

            rgb = vec4(light.color.xyz * light.params.x * result * max(factor, 0.0), 1.0);
        } else {
            rgb = vec4(vec3(0.0), 1.0);
        }
    } else {
        rgb = vec4(vec3(0.0), 1.0);
    }
}
