#version 450 core

#include "ShaderLayout.h"
#include "BRDF.h"

layout(location = 50) uniform sampler2D normalsMap;
layout(location = 51) uniform sampler2D diffuseMap;
layout(location = 52) uniform sampler2D paramsMap;
layout(location = 53) uniform sampler2D depthMap;
layout(location = 54) uniform sampler2D shadowMap;

layout(location = 0) in vec4 _vertex;

layout(location = 0) out vec4 rgb;

void main (void) {
    vec2 proj   = ((_vertex.xyz / _vertex.w) * 0.5 + 0.5).xy;

    vec4 slice0 = texture( normalsMap,  proj );

    // Light model LIT
    if(slice0.w > 0.33) {
        float depth = texture( depthMap, proj ).x;
        vec3 world = getWorld( camera.screenToWorld, proj, depth );

        vec3 n = normalize( slice0.xyz * 2.0 - 1.0 );

        vec3 dir = light.position - world;
        vec3 l = normalize(dir);
        float dist  = length(dir);

        float spot  = dot(l, light.direction);
        float fall = 0.0;
        if(spot > light.params.w) {
            fall  = 1.0 - (1.0 - spot) / (1.0 - light.params.w);
            fall  = getAttenuation( dist, light.params.y ) * light.params.x * fall;
        }

        vec4 slice1 = texture( paramsMap, proj );
        float rough = slice1.x;
        float metal = slice1.z;
        float spec  = slice1.w;

        vec4 slice2 = texture( diffuseMap, proj );
        vec3 albedo = slice2.xyz;

        vec3 v = normalize( camera.position.xyz - world );
        vec3 h = normalize( l + v );

        float cosTheta = clamp(dot(l, n), 0.0, 1.0);

        float shadow = 1.0;
        if(light.shadows == 1.0) {
            vec4 offset = light.tiles[0];
            vec4 proj   = light.matrix[0] * vec4(world, 1.0);
            vec3 coord  = (proj.xyz / proj.w);
            shadow  = getShadow(shadowMap, (coord.xy * offset.zw) + offset.xy, coord.z - light.bias.x);
        }

        vec3 refl = mix(vec3(spec), albedo, metal) * getCookTorrance(n, v, h, cosTheta, rough);
        vec3 result = albedo * (1.0 - metal) + refl;
        float diff = max(PI * getLambert(cosTheta, light.params.x) * fall * shadow, 0.0);

        rgb = vec4(light.color.xyz * result * diff, 1.0);
    } else {
        rgb = vec4(vec3(0.0), 1.0);
    }
}
