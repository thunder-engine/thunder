#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;
layout(location = 6) in vec4 _color;

layout(location = 7) in vec3 _view;

layout(location = 0) out vec4 gbuffer1;
#ifdef BLEND_OPAQUE
layout(location = 1) out vec4 gbuffer2;
layout(location = 2) out vec4 gbuffer3;
layout(location = 3) out vec4 gbuffer4;
#endif

#pragma uniforms

#pragma functions

void main(void) {
#pragma fragment

#ifdef SIMPLE
    float alpha = Opacity;
    if(g.clip >= alpha) {
        discard;
    }
    gbuffer1 = l.color;
#elif LIGHT
    gbuffer1 = vec4(Emissive, 1.0);
#else
    vec3 emit = Emissive * l.color.xyz;
    float alpha = Opacity * l.color.w;

    #ifdef BLEND_OPAQUE
    if(g.clip >= alpha) {
        discard;
    }
    vec3 norm = vec3(1.0);
    vec3 matv = vec3(0.0, 0.0, Metallic);
    float model = 0.0;
    #ifdef MODEL_LIT
    vec3 normal = Normal * 2.0 - 1.0;
    normal = normalize(normal.x * _t + normal.y * _b + normal.z * _n);

    model = 0.34;
    norm = normal * 0.5 + 0.5;
    matv.x = max(0.01, Roughness);
    #endif

    vec3 albd = Diffuse * l.color.xyz;

    float spec = 1.0;
    gbuffer1 = vec4(emit, 0.0);
    gbuffer2 = vec4(norm, model);
    gbuffer3 = vec4(albd, 1.0);
    gbuffer4 = vec4(matv, spec);
    #else
    gbuffer1 = vec4(emit, alpha);
    #endif
#endif
}
