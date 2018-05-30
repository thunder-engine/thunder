#pragma version

#include ".embedded/Common.vert"
#include ".embedded/BRDF.frag"

uniform sampler2D normalsMap;
uniform sampler2D diffuseMap;
uniform sampler2D paramsMap;
uniform sampler2D emissiveMap;
uniform sampler2D depthMap;
uniform sampler2D shadowMap;

layout(location = 1) in vec2 _uv0;

out vec4    rgb;

void main (void) {
    vec4 slice0 = texture( normalsMap,  _uv0 );
    vec4 slice2 = texture( paramsMap,   _uv0 );
    vec3 emit   = texture( emissiveMap, _uv0 ).xyz;

    vec3 n      = normalize( 2.0 * slice0.xyz - vec3( 1.0 ) );
    float ln    = dot( transform.orientation, n );

    // Light model LIT
    if(slice0.w > 0.33) {
        float depth = texture( depthMap, _uv0 ).x;
        vec4 world  = getWorld( camera.mvpi, _uv0, depth );

        vec4 slice1 = texture( diffuseMap, _uv0 );
        float rough = max( 0.01, slice1.w );
        float spec  = slice2.w;
        float metal = slice2.z;

        vec3 albedo = slice1.xyz;
        vec3 v      = normalize( camera.position.xyz - (world / world.w).xyz );
        vec3 h      = normalize( transform.orientation + v );

        vec3 refl   = mix(vec3(spec), albedo, metal) * getCookTorrance( n, v, h, ln, rough );
        vec3 color  = albedo * (1.0 - metal) + refl;
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

        rgb = vec4( light.color.xyz * color * shadow * diff + emit, 1.0 );
    } else {
        rgb = vec4( emit, 1.0 );
    }
}
