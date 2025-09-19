#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec4 _color;

#ifdef USE_TBN
    layout(location = 3) out vec3 _n;
    layout(location = 4) out vec3 _t;
    layout(location = 5) out vec3 _b;
#endif

layout(location = 6) out vec3 _view;
layout(location = 7) flat out vec4 _objectId;
layout(location = 8) flat out int _instanceOffset;
layout(location = 9) out mat4 _modelView;

#pragma vertexFunctions

void main(void) {
#pragma offset

    _modelView = g.view;

    vec3 camera = vec3(g.view[0].w,
                       g.view[1].w,
                       g.view[2].w);

    vec3 PositionOffset = vec3(0.0f);

#pragma vertex

    vec4 v0 = instance.data[_instanceOffset];
    vec4 v1 = instance.data[_instanceOffset + 1];
    vec4 v2 = instance.data[_instanceOffset + 2];
    vec4 v3 = instance.data[_instanceOffset + 3];

    vec3 worldPosition = v2.xyz;
    vec3 sizeRot = v0.xyz;
    vec2 uvScale = vec2(1.0f / v1.x, 1.0f / v1.y);
    vec2 uvOffset = vec2(mod(v1.z, v1.x) * uvScale.x, floor(v1.z / v1.x) * uvScale.y);

    sizeRot.x *= vertex.x;
    sizeRot.y *= vertex.y;

    _objectId = vec4(v0.w, v1.w, v2.w, v3.w);

    float angle = sizeRot.z;
    float x = cos(angle) * sizeRot.x + sin(angle) * sizeRot.y;
    float y = sin(angle) * sizeRot.x - cos(angle) * sizeRot.y;

    vec3 target = g.cameraTarget.xyz;
    if(g.cameraProjection[2].w < 0.0f) {
        target = worldPosition.xyz;
    }

    vec3 normal = normalize(g.cameraPosition.xyz - target);
    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), normal));
    vec3 up = normalize(cross(normal, right));

    vec3 v = (up * x + right * y) + worldPosition + PositionOffset;

    _vertex = g.projection * (_modelView * vec4(v, 1.0f));
    _view = normalize(v - camera);

#ifdef USE_TBN
    _n = vec3(0.0f);
    _t = vec3(0.0f);
    _b = vec3(0.0f);
#endif
#ifdef ORIGIN_TOP
    _vertex.y = -_vertex.y;
#endif
    _color = color;
    _uv0 = uv0 * uvScale.xy + uvOffset.xy;
    gl_Position = _vertex;
}
