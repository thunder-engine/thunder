#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

#ifdef USE_TBN
    layout(location = 3) in vec3 _n;
    layout(location = 4) in vec3 _t;
    layout(location = 5) in vec3 _b;    
#endif

layout(location = 6) in vec3 _view;
layout(location = 7) flat in vec4 _objectId;
layout(location = 8) flat in int _instanceOffset;
layout(location = 9) in mat4 _modelView;

layout(location = 0) out vec4 gbuffer0;
#ifdef USE_GBUFFER
    layout(location = 1) out vec4 gbuffer1;
    layout(location = 2) out vec4 gbuffer2;
    layout(location = 3) out vec4 gbuffer3;
#endif

#pragma uniforms

#include "Functions.h"

#pragma functions

void main(void) {
#pragma instance

    vec3 Diffuse;
    vec3 Emissive;
    vec3 Normal;
    float Metallic;
    float Roughness;
    float Opacity;
    float IOR;

#pragma fragment

    float alpha = Opacity * _color.w;
    if(g.clip >= alpha) {
        discard;
    }

#ifdef VISIBILITY_BUFFER
    gbuffer0 = _objectId;
    return;
#endif

    vec3 emit = Emissive * _color.xyz;

#ifdef USE_GBUFFER
    float model = 0.0f;
    vec3 normal = vec3(0.5f, 0.5f, 1.0f);
    vec3 albedo = Diffuse * _color.xyz;

    #ifdef USE_TBN
        normal = Normal * 2.0f - 1.0f;
        normal = normalize(normal.x * _t + normal.y * _b + normal.z * _n);

        emit += mix(albedo, vec3(0.0f), Metallic);
        model = 0.333f;
        normal = normal * 0.5f + 0.5f;
        Roughness = max(0.01f, Roughness);
    #endif

    gbuffer0 = vec4(emit, 0.0f); // Accumulation buffer
    gbuffer1 = vec4(normal, model);
    gbuffer2 = vec4(albedo, model);
    gbuffer3 = vec4(Roughness, 0.0f, Metallic, 1.0f); // Variables

#else
    gbuffer0 = vec4(emit, alpha);

#endif

}
