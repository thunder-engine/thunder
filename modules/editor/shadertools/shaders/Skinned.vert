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

layout(location = 5) in vec4 skinnedBones;
layout(location = 6) in vec4 skinnedWeights;

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

    vec4 bones = skinnedBones;
    vec4 weights = skinnedWeights;
    vec4 finalVector = vec4(0.0);
#ifdef USE_TBN
    _n = vec3(0.0);
    _t = vec3(0.0);
#endif
    for(int i = 0; i < 4; i++) {
        if(weights.x > 0.0) {
#pragma skinOffset

            int index = _instanceOffset + skinOffset + int(bones.x) * 3;

            vec4 m1 = vec4(instance.data[index]);
            vec4 m2 = vec4(instance.data[index + 1]);
            vec4 m3 = vec4(instance.data[index + 2]);

            mat4 m44 = mat4(vec4(m1.xyz, 0.0),
                            vec4(m2.xyz, 0.0),
                            vec4(m3.xyz, 0.0),
                            vec4(m1.w, m2.w, m3.w, 1.0));

            finalVector += (m44 * vec4(vertex, 1.0)) * weights.x;

            #ifdef USE_TBN
                mat3 m33 = mat3(m44[0].xyz,
                                m44[1].xyz,
                                m44[2].xyz);

                _n += (m33 * normal) * weights.x;
                _t += (m33 * tangent) * weights.x;
            #endif

            bones = bones.yzwx;
            weights = weights.yzwx;
        }
    }

    #ifdef USE_TBN
        mat3 rot = mat3(modelMatrix);
        _t = normalize(rot * _t);
        _n = normalize(rot * _n);
        _b = cross(_t, _n);
    #endif

    vec3 v = finalVector.xyz / finalVector.w + PositionOffset;
    _vertex = g.projection * (_modelView * vec4(v, 1.0));
    _view = normalize(v - g.cameraPosition.xyz);

#ifdef ORIGIN_TOP
    _vertex.y = -_vertex.y;
#endif

    _color = color;
    _uv0 = uv0;
    gl_Position = _vertex;
}
