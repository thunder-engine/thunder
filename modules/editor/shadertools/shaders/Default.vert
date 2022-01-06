#version 450 core

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec4 color;

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec2 _uv1;
layout(location = 3) out vec3 _n;
layout(location = 4) out vec3 _t;
layout(location = 5) out vec3 _b;
layout(location = 6) out vec4 _color;

void main(void) {
    mat4 mv = g.view * l.model;
    mat3 rot = mat3(l.model);

    _t = normalize(rot * tangent);
    _n = normalize(rot * normal);
    _b = cross ( _t, _n );
    _color = color;
    _uv0 = uv0;
    _vertex = g.projection * (mv * vec4(vertex, 1.0));

    gl_Position = _vertex;
}
