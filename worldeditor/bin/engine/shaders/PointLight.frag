#pragma version

#include ".embedded/Common.vert"
#include ".embedded/BRDF.frag"

uniform sampler2D normalsMap;
uniform sampler2D diffuseMap;
uniform sampler2D paramsMap;
uniform sampler2D emissiveMap;
uniform sampler2D depthMap;
uniform sampler2D shadowMap;

layout(location = 0) in vec4 _vertex;

//varying vec3    light_dir;

out vec4    rgb;

void main (void) {
    vec3 proj   = 0.5 * ( _vertex.xyz / _vertex.w ) + 0.5;

    vec4 slice0 = texture( normalsMap,   proj.xy );
    vec4 slice2 = texture( paramsMap,    proj.xy );
    vec3 emit   = texture( emissiveMap,  proj.xy ).xyz;

    vec3 n      = normalize( 2.0 * slice0.xyz - vec3( 1.0 ) );
    float ln    = dot( light.position.xyz, n );

    // Light model LIT
    if(slice0.w > 0.33) {
        float depth = texture( depthMap,     proj.xy ).x;
        vec4 world  = getWorld( camera.mvpi, proj.xy, depth );

        vec3 pos    = light.position.xyz * depth / light.position.z;
        float dist  = length( (world.xyz / world.z) - pos );
        float fall  = getAttenuation( dist, light.radius ) * light.brightness;

        vec4 slice1 = texture( diffuseMap, proj.xy );
        float rough = max( 0.01, slice1.w );
        float spec  = slice2.w;
        float metal = slice2.z;

        vec3 albedo = slice1.xyz;
        vec3 v      = normalize( camera.position.xyz - (world / world.w).xyz );
        vec3 h      = normalize( light.position.xyz + v );

        vec3 refl   = mix(vec3(spec), albedo, metal) * getCookTorrance( n, v, h, ln, rough );
        vec3 result = vec3(1.0);//albedo * (1.0 - metal) + refl;

        float diff  = getLambert( ln, light.brightness );

        float shadow    = 1.0;


        rgb = vec4((world.xyz / world.z), 1.0);
        //rgb = vec4( light.color.xyz * result * shadow * diff + emit, 1.0 );
    } else {
        rgb = vec4( emit, 1.0 );
    }
}
