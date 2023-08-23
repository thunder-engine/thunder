#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;
#ifdef MODEL_LIT
    layout(location = 3) in vec3 normal;
    layout(location = 4) in vec3 tangent;
#endif

layout(location = 7) in vec4 particlePosRot;
layout(location = 8) in vec4 particleSizeDist;
layout(location = 9) in vec4 particleRes1;
layout(location = 10) in vec4 particleRes2;

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec4 _color;
#ifdef MODEL_LIT
    layout(location = 3) out vec3 _n;
    layout(location = 4) out vec3 _t;
    layout(location = 5) out vec3 _b;
#endif
layout(location = 6) out vec3 _view;
layout(location = 7) out mat4 _modelView;

#pragma uniforms

#pragma functions

void main(void) {
    mat4 model = l.model;

    _modelView = g.view * model;

    vec3 camera = vec3(g.view[0].w,
                       g.view[1].w,
                       g.view[2].w);

#pragma vertex

    float angle = particlePosRot.w;  // rotation
    float x = cos(angle) * vertex.x + sin(angle) * vertex.y;
    float y = sin(angle) * vertex.x - cos(angle) * vertex.y;

    vec3 target = g.cameraTarget.xyz;
    if(g.cameraProjection[2].w < 0.0) {
        target = particlePosRot.xyz;
    }
    vec3 normal = normalize(g.cameraPosition.xyz - target);
    vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), normal));
    vec3 up = normalize(cross(normal, right));

    vec4 v = vec4((up * x + right * y) * particleSizeDist.xyz + particlePosRot.xyz + PositionOffset, 1.0);
    #ifdef MODEL_LIT
        _t = tangent;
        _n = normal;
        _b = cross(_t, _n);
    #endif
    _vertex = g.projection * (_modelView * v);
    _view = normalize((model * v).xyz - camera);

    _color = color;
    _uv0 = uv0;
    gl_Position = _vertex;
}
