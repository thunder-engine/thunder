#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec4 color;

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec4 _color;

layout(location = 6) out vec3 _view;
layout(location = 7) out vec4 _objectId;
layout(location = 8) out int _instanceOffset;
layout(location = 9) out mat4 _modelView;

#pragma functions

void main(void) {
#pragma offset

    _modelView = g.view;

    vec3 camera = vec3(g.view[0].w,
                       g.view[1].w,
                       g.view[2].w);

    vec3 PositionOffset = vec3(0.0f);

#pragma vertex

    vec4 worldPosition = instance.data[_instanceOffset];
    vec4 sizeRot = instance.data[_instanceOffset + 1];
    vec4 uvScaleDist = instance.data[_instanceOffset + 2];
    vec4 uvOffset = instance.data[_instanceOffset + 3];

    _objectId = vec4(worldPosition.w, sizeRot.w, uvScaleDist.w, uvOffset.w);

    float angle = sizeRot.z;
    float x = cos(angle) * vertex.x + sin(angle) * vertex.y;
    float y = sin(angle) * vertex.x - cos(angle) * vertex.y;

    vec3 target = g.cameraTarget.xyz;
    if(g.cameraProjection[2].w < 0.0f) {
        target = worldPosition.xyz;
    }

    vec3 normal = normalize(g.cameraPosition.xyz - target);
    vec3 right = normalize(cross(vec3(0.0f, 1.0f, 0.0f), normal));
    vec3 up = normalize(cross(normal, right));

    vec3 v = (up * x + right * y) * vec3(sizeRot.xy, 1.0f) + worldPosition.xyz + PositionOffset;

    _vertex = g.projection * (_modelView * vec4(v, 1.0f));
    _view = normalize(v - camera);

    _color = color;
    _uv0 = uv0 * uvScaleDist.xy + uvOffset.xy;
    gl_Position = _vertex;
}
