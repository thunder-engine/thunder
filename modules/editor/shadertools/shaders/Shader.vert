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

#ifdef TYPE_SKINNED
    layout(location = 5) in vec4 skinnedBones;
    layout(location = 6) in vec4 skinnedWeights;
#endif

#ifdef INSTANCING
    layout(location = 7) in mat4 instanceMatrix;
#else
    #ifdef TYPE_BILLBOARD
    layout(location = 7) in vec4 particlePosRot;
    layout(location = 8) in vec4 particleSizeDist;
    layout(location = 9) in vec4 particleRes1;
    layout(location = 10) in vec4 particleRes2;
    #endif
#endif

#ifdef TYPE_SKINNED
layout(location = 6) uniform sampler2D skinMatrices;
layout(location = 7) uniform vec4 skinParams;
#endif

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 3) out vec4 _color;
#ifdef MODEL_LIT
    layout(location = 4) out vec3 _n;
    layout(location = 5) out vec3 _t;
    layout(location = 6) out vec3 _b;
#endif

layout(location = 7) out vec3 _view;

#pragma uniforms

#pragma functions

void main(void) {
#ifdef TYPE_FULLSCREEN
    _vertex = vec4(vertex * 2.0, 1.0);
#else
    #ifdef INSTANCING
        mat4 model = instanceMatrix;
    #else
        mat4 model = l.model;
    #endif
        mat4 mv = g.view * model;

        vec3 camera = vec3(g.view[0].w,
                           g.view[1].w,
                           g.view[2].w);

    #pragma vertex

    #ifdef TYPE_STATIC
        #ifdef MODEL_LIT
            mat3 rot = mat3(model);
            _t = normalize(rot * tangent);
            _n = normalize(rot * normal);
            _b = cross(_t, _n);
        #endif
        vec4 v = vec4(vertex + PositionOffset, 1.0);
        _vertex = g.projection * (mv * v);
        _view = normalize((model * v).xyz - g.cameraPosition.xyz);
    #endif

    #ifdef TYPE_BILLBOARD
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
        _vertex = g.projection * (mv * v);
        _view = normalize((model * v).xyz - camera);
    #endif

    #ifdef TYPE_SKINNED
        vec4 bones = skinnedBones;
        vec4 weights = skinnedWeights;
        vec4 finalVector = vec4(0.0);
        for(int i = 0; i < 4; i++) {
            if(weights.x > 0.0) {
                float width = 1.0 / 512.0;
                int x = int(bones.x) * 3; // index
                int y = 0;

                vec4 m1 = texture(skinMatrices, vec2(x,     y) * width);
                vec4 m2 = texture(skinMatrices, vec2(x + 1, y) * width);
                vec4 m3 = texture(skinMatrices, vec2(x + 2, y) * width);

                mat4 m44 = mat4(vec4(m1.xyz, 0.0),
                                vec4(m2.xyz, 0.0),
                                vec4(m3.xyz, 0.0),
                                vec4(m1.w, m2.w, m3.w, 1.0));

                finalVector += (m44 * vec4(vertex, 1.0)) * weights.x;
                #ifdef MODEL_LIT
                    mat3 m33 = mat3(m44[0].xyz,
                                    m44[1].xyz,
                                    m44[2].xyz);

                    _n += m33 * normal * weights.x;
                    _t += m33 * tangent * weights.x;
                #endif
                bones = bones.yzwx;
                weights = weights.yzwx;
            }
        }
        #ifdef MODEL_LIT
            _b = cross(_t, _n);
        #endif
        vec3 v = finalVector.xyz / finalVector.w + PositionOffset;
        _vertex = g.projection * (g.view * vec4(v, 1.0));
        _view = normalize(v - g.cameraPosition.xyz);
    #endif
#endif

    gl_Position = _vertex;
    _color = color;
    _uv0   = uv0;
}
