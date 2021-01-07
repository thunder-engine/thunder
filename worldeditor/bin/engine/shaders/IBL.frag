#pragma version

#include "Common.vert"

layout(location = 50) uniform sampler2D depthMap;
layout(location = 51) uniform sampler2D normalsMap;
layout(location = 52) uniform sampler2D paramsMap;
layout(location = 53) uniform sampler2D rgbMap;
layout(location = 54) uniform samplerCubeArray environmentMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 7) in vec3 _view;

layout(location = 0) out vec3 color;

vec3 boxProjection(vec3 dir) {
    return dir;
}

void main(void) {
    float depth = texture(depthMap, _uv0).x;
    if(depth < 1.0) {
        vec3 origin = vec3(_uv0, depth);
        vec3 world = getWorld(camera.screenToWorld, origin.xy, origin.z);

        vec3 v = normalize(world - camera.position.xyz);
        vec3 n = texture(normalsMap, _uv0).xyz * 2.0 - 1.0;
        vec3 refl = reflect(v, n);
        
        refl = boxProjection(refl);
        
        vec4 params = texture(paramsMap, _uv0);
        float rough = params.x;

        vec3 ibl = textureLod(environmentMap, vec4(refl, 0.0), rough * 10.0).xyz;

        vec4 sslr = texture(rgbMap, _uv0);
        color = mix(ibl, sslr.xyz, sslr.w) * (1.0 - rough);
    } else {
        color = vec3(0.0);
    }
}
