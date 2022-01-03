#version 450 core

#include "ShaderLayout.h"
#include "BRDF.h"

layout(binding = UNIFORM) uniform Uniforms {
    mat4 matrix[4];
    vec4 tiles[4];
    vec4 color;
    vec4 lod;
    vec4 params; // x - brightness, y - radius/width, z - length/height, w - cutoff
    vec4 bias;
    vec4 direction;
    float shadows;
} uni;

layout(binding = UNIFORM + 1) uniform sampler2D normalsMap;
layout(binding = UNIFORM + 2) uniform sampler2D diffuseMap;
layout(binding = UNIFORM + 3) uniform sampler2D paramsMap;
layout(binding = UNIFORM + 4) uniform sampler2D depthMap;
layout(binding = UNIFORM + 5) uniform sampler2D shadowMap;

layout(location = 0) in vec4 _vertex;

layout(location = 0) out vec4 rgb;

void main (void) {
    vec2 proj   = ((_vertex.xyz / _vertex.w) * 0.5 + 0.5).xy;

    vec4 slice0 = texture(normalsMap,  proj);

    // Light model LIT
    if(slice0.w > 0.33) {
        float depth = texture(depthMap, proj).x;
        vec3 world  = getWorld(g.cameraScreenToWorld, proj, depth);

        vec3 n = normalize(slice0.xyz * 2.0 - 1.0);

        vec4 slice1 = texture(paramsMap, proj);
        float rough = slice1.x;
        float metal = slice1.z;
        float spec  = slice1.w;

        vec4 slice2 = texture(diffuseMap, proj);
        vec3 albedo = slice2.xyz;

        vec3 v = normalize(g.cameraPosition.xyz - world);
        vec3 h = normalize(uni.direction.xyz + v);

        float cosTheta = clamp(dot(uni.direction.xyz, n), 0.0, 1.0);

        float shadow = 1.0;
        if(uni.shadows == 1.0) {
            int index = 3;
            float bias = uni.bias.w;
            if(uni.lod.x > depth) {
                index = 0;
                bias = uni.bias.x;
            } else if(uni.lod.y > depth) {
                index = 1;
                bias = uni.bias.y;
            } else if(uni.lod.z > depth) {
                index = 2;
                bias = uni.bias.z;
            }

            vec4 offset = uni.tiles[index];
            vec4 proj   = uni.matrix[index] * vec4(world, 1.0);
            vec3 coord  = proj.xyz / proj.w;
            if(coord.x > 0.0 && coord.x < 1.0 && coord.y > 0.0 && coord.y < 1.0 && coord.z > 0.0 && coord.z < 1.0) {
                shadow  = getShadow(shadowMap, (coord.xy * offset.zw) + offset.xy, coord.z - bias);
            }
        }

        vec3 refl = mix(vec3(spec), albedo, metal) * getCookTorrance(n, v, h, cosTheta, rough);
        vec3 result = albedo * (1.0 - metal) + refl;
        float diff = getLambert(cosTheta, uni.params.x) * shadow;

        rgb = vec4(uni.color.xyz * result * diff, 1.0);
        return;
    }
    rgb = vec4(vec3(0.0), 1.0);
}
