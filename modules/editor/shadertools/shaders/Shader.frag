#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
#ifdef MODEL_LIT
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;
#endif

layout(location = 7) in vec3 _view;

layout(location = 0) out vec4 gbuffer0;
#ifdef BLEND_OPAQUE
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;
layout(location = 3) out vec4 gbuffer3;
#endif

#pragma uniforms

#pragma functions

void main(void) {
#pragma fragment

    float alpha = Opacity * l.color.w;
    if(g.clip >= alpha) {
        discard;
    }

    vec3 emit = Emissive * l.color.xyz;

#ifdef VISIBILITY_BUFFER
    gbuffer0 = l.objectId;
    return;
#endif

#ifndef BLEND_OPAQUE
    gbuffer0 = vec4(emit, alpha);
#endif

#ifdef BLEND_OPAQUE
    float model = 0.0f;
    vec3 normal = vec3(1.0f);
    vec3 albedo = Diffuse * l.color.xyz;

    #ifdef MODEL_LIT
        normal = Normal * 2.0f - 1.0f;
        normal = normalize(normal.x * _t + normal.y * _b + normal.z * _n);

        vec3 radianceCache = texture(radianceMap, cartesianToSpherical(normal)).xyz;
        emit += mix(albedo * radianceCache, vec3(0.0f), Metallic);
        model = 0.333f;
        normal = normal * 0.5f + 0.5f;
        Roughness = max(0.01f, Roughness);
    #endif

    gbuffer0 = vec4(emit, 0.0f); // Accumulation buffer
    gbuffer1 = vec4(normal, model);
    gbuffer2 = vec4(albedo, model);
    gbuffer3 = vec4(Roughness, 0.0f, Metallic, 1.0f); // Variables
#endif

}
