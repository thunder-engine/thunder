#pragma version

#include "Common.vert"
#include "BRDF.frag"

layout(location = 32) uniform sampler2D normalsMap;
layout(location = 34) uniform sampler2D paramsMap;
layout(location = 35) uniform sampler2D depthMap;
layout(location = 36) uniform sampler2D shadowMap;

layout(location = 0) in vec4 _vertex;

layout(location = 0) out vec4 rgb;

void main (void) {
    vec2 proj   = ((_vertex.xyz / _vertex.w) * 0.5 + 0.5).xy;

    vec4 slice0 = texture( normalsMap,  proj );
    vec3 n      = normalize( slice0.xyz * 2.0 - 1.0 );

    float ln    = dot( light.direction, n );

    // Light model LIT
    if(slice0.w > 0.33) {
        float depth = texture( depthMap, proj ).x;
        vec4 world  = getWorld( camera.mvpi, proj, depth );

        vec4 slice1 = texture( paramsMap,   proj );
        float rough = slice1.x;
        float metal = slice1.z;
        float spec  = slice1.w;

        vec3 v      = normalize( camera.position.xyz - (world / world.w).xyz );
        vec3 h      = normalize( light.direction + v );

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
            vec3 coord  = proj.xyz / proj.w;
            if(coord.x > 0.0 && coord.x < 1.0 && coord.y > 0.0 && coord.y < 1.0 && coord.z > 0.0 && coord.z < 1.0) {
                shadow  = getShadow(shadowMap, (coord.xy * offset.zw) + offset.xy, coord.z - light.bias);
            }
        }

        float refl = getCookTorrance(n, v, h, ln, rough) * spec;
        float diff = getLambert(ln, light.params.x) * shadow;

        rgb = vec4(light.color.xyz * (diff + refl), 1.0);
    } else {
        rgb = vec4(vec3(0.0), 1.0);
    }
}
