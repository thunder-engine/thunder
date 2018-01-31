#pragma version

#include ".embedded/Common.vert"
#include ".embedded/BRDF.frag"

uniform sampler2D layer0;
uniform sampler2D layer1;
uniform sampler2D layer2;
uniform sampler2D layer3;
uniform sampler2D depthMap;
uniform sampler2D shadowMap;

layout(location = 1) in vec2 _uv0;

out vec4    rgb;

float getShadowSample(sampler2D map, vec2 coord, float t) {
    return step(t, texture(map, coord).x);
}

float linstep(float l, float h, float v) {
    return clamp((v-l)/(h-l), 0.0, 1.0);
}

float getShadowVarianceSample(sampler2D map, vec2 coord, float t) {
    vec2 m  = texture(map, coord).xy;

    float p = step(t, m.x);
    float v = max(m.y - m.x * m.x, 0.000002); // 0.000002 = Min variance

    float d = t - m.x;
    float pm = linstep(0.2, 1.0, v / (v + d * d));

    return clamp(max(p, pm), 0.0, 1.0);
}

void main (void) {
    vec4 slice0 = texture( layer0, _uv0 );
    vec3 emit   = texture( layer3, _uv0 ).xyz;

    vec3 n      = normalize( 2.0 * slice0.xyz - vec3( 1.0 ) );
    float ln    = dot( light.dir, n );

    // Light model LIT
    if(slice0.w > 0.33) {
        float depth     = texture( depthMap, _uv0 ).x;
        vec4 world      = getWorld(transform.mvpi, _uv0, depth);

        float shadow    = 1.0;
        if(light.shadows == 1.0) {
            int index   = 0;

            vec4 proj   = light.matrix[index] * ( world / world.w );
            vec3 coord  = proj.xyz / proj.w;

            if(coord.x > 0.0 && coord.y > 0.0 && coord.z > 0.0 && 1.0 > coord.x && 1.0 > coord.y && 1.0 > coord.z) {
                shadow  = getShadowVarianceSample(shadowMap, coord.xy, coord.z);
            }
        }

        vec4 slice1 = texture( layer1, _uv0 );
        vec4 slice2 = texture( layer2, _uv0 );
        float rough = max( 0.01, slice2.w );
        float spec  = slice2.y;
        float metal = slice2.z;

        vec3 albedo = slice1.xyz;
        vec3 v      = normalize( camera.position.xyz - ( world / world.w ).xyz );
        vec3 h      = normalize( light.dir + v );

        vec3 refl   = mix(vec3(spec), albedo, metal) * getCookTorrance( n, v, h, ln, rough ) ;
        vec3 color  = albedo * (1.0 - metal) + refl;

        float diff  = getLambert( ln, light.brightness );

        rgb = vec4( light.color * color * shadow * diff + emit, 1.0 );
    } else {
        rgb = vec4( emit, 1.0 );
    }
}
/*
    if(1.0 - (1.0 / (light.lod.x * 10.0)) > depth) {
        index   = 0;
    } else if(1.0 - (1.0 / (light.lod.y * 10.0)) > depth) {
        index   = 1;
    } else if(1.0 - (1.0 / (light.lod.z * 10.0)) > depth) {
        index   = 2;
    }
*/
