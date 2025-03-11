#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec4 _color;

void main(void) {
    _vertex = vec4(vertex * 2.0, 1.0);
    _uv0 = uv0;
    _color = color;
#ifdef ORIGIN_TOP
    _uv0.y = 1.0 - _uv0.y;
#endif
    gl_Position = _vertex;
}
