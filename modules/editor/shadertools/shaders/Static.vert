#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

#ifdef USE_TBN
    layout(location = 3) in vec3 normal;
    layout(location = 4) in vec3 tangent;
#endif

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec4 _color;

#ifdef USE_TBN
    layout(location = 3) out vec3 _n;
    layout(location = 4) out vec3 _t;
    layout(location = 5) out vec3 _b;
#endif

layout(location = 6) out vec3 _view;
layout(location = 7) out vec4 _objectId;
layout(location = 8) out int _instanceOffset;
layout(location = 9) out mat4 _modelView;

#pragma functions

void main(void) {
#pragma offset

#pragma instance

#pragma objectId

    _modelView = g.view * modelMatrix;

    vec3 camera = vec3(g.view[0].w,
                       g.view[1].w,
                       g.view[2].w);

    vec3 PositionOffset = vec3(0.0f);

#pragma vertex

    #ifdef USE_TBN
        mat3 rot = mat3(modelMatrix);
        _t = normalize(rot * tangent);
        _n = normalize(rot * normal);
        _b = cross(_t, _n);
    #endif
    vec4 v = vec4(vertex + PositionOffset, 1.0);
    _vertex = g.projection * (_modelView * v);
    _view = normalize((modelMatrix * v).xyz - g.cameraPosition.xyz);

    _color = color;
    _uv0 = uv0;
    gl_Position = _vertex;
}
