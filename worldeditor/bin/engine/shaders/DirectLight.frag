#pragma version

#include "Common.vert"
#include "BRDF.frag"

layout(location = 30) uniform sampler2D normalsMap;
layout(location = 31) uniform sampler2D diffuseMap;
layout(location = 32) uniform sampler2D paramsMap;
layout(location = 33) uniform sampler2D emissiveMap;
layout(location = 34) uniform sampler2D depthMap;
layout(location = 35) uniform sampler2D shadowMap;

layout(location = 0) in vec4 _vertex;

layout(location = 0) out vec4 rgb;

void main (void) {
    vec2 proj   = (0.5 * ( _vertex.xyz / _vertex.w ) + 0.5).xy;

    vec4 slice0 = texture( normalsMap,  proj );
    vec4 slice2 = texture( paramsMap,   proj );
    vec3 emit   = texture( emissiveMap, proj ).xyz;

    vec3 n      = normalize( 2.0 * slice0.xyz - vec3( 1.0 ) );
    float ln    = dot( light.position.xyz, n );

    // Light model LIT
    if(slice0.w > 0.33) {
        float depth = texture( depthMap, proj ).x;
        vec4 world  = getWorld( camera.mvpi, proj, depth );

        vec4 slice1 = texture( diffuseMap, proj );
        float rough = max( 0.01, slice1.w );
        float spec  = slice2.w;
        float metal = slice2.z;

        vec3 albedo = slice1.xyz;
        vec3 v      = normalize( camera.position.xyz - (world / world.w).xyz );
        vec3 h      = normalize( light.position.xyz + v );

        vec3 refl   = mix(vec3(spec), albedo, metal) * getCookTorrance( n, v, h, ln, rough );
        vec3 result = albedo * (1.0 - metal) + refl;
        float diff  = getLambert( ln, light.brightness );

        float shadow    = 1.0;
        if(light.shadows == 1.0) {
            int index   = 3;
            if(light.lod.x > depth) {
                index   = 0;
            } else if(light.lod.y > depth) {
                index   = 1;
            } else if(light.lod.z > depth) {
                index   = 2;
            }

            vec4 offset = light.tiles[index];
            vec4 proj   = light.matrix[index] * world;
            vec3 coord  = (proj.xyz / proj.w);
            if(coord.x > 0.0 && coord.x < 1.0 && coord.y > 0.0 && coord.y < 1.0 && coord.z > 0.0 && coord.z < 1.0) {
                shadow  = getShadow(shadowMap, (coord.xy * offset.zw) + offset.xy, coord.z - light.bias);
            }
        }

        rgb = vec4( light.color.xyz * result * shadow * diff + emit, 1.0 );
    } else {
        rgb = vec4( emit, 1.0 );
    }
}
