#version 450 core

#pragma flags

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

layout(location = 6) flat out vec4 _objectId;
layout(location = 7) flat out int _instanceOffset;

#include "ShaderLayout.h"

#pragma vertexFunctions

void main(void) {
#pragma offset
#pragma instance

#pragma objectId

    vec3 PositionOffset = vec3(0.0f);

#pragma vertex

    mat4 _modelMatrix = modelMatrix();
    #ifdef USE_TBN
        mat3 rot = mat3(_modelMatrix);
        _t = normalize(rot * tangent);
        _n = normalize(rot * normal);
        _b = cross(_t, _n);
    #endif
    _vertex = _modelMatrix * vec4(vertex + PositionOffset, 1.0);
    vec4 pos = cameraWorldToScreen() * _vertex;

#ifdef ORIGIN_TOP
    pos.y = -pos.y;
#endif
    _color = color;
    _uv0 = uv0;
    gl_Position = pos;
}
