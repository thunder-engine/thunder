#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec4 color;

#ifdef INSTANCING
    layout(location = 8) in mat4 instanceMatrix;
#else
    #ifdef TYPE_BILLBOARD
    layout(location = 8) in vec4 particlePosRot;
    layout(location = 9) in vec4 particleSizeDist;
    layout(location = 10) in vec4 particleRes1;
    layout(location = 11) in vec4 particleRes2;
    #else
    layout(location = 5) in vec2 uv1;
    #endif
#endif

#ifdef TYPE_SKINNED
    layout(location = 6) in vec4 bones;
    layout(location = 7) in vec4 weights;

    layout(location = 6) uniform sampler2D skinMatrices;
    layout(location = 7) uniform vec4 skinParams;
#endif

layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec2 _uv1;
layout(location = 3) out vec3 _n;
layout(location = 4) out vec3 _t;
layout(location = 5) out vec3 _b;
layout(location = 6) out vec4 _color;

layout(location = 7) out vec3 _view;

#pragma uniforms

struct Vertex {
    vec3 v;
    vec3 t;
    vec3 n;
};

#ifdef TYPE_STATIC
Vertex staticMesh(vec3 v, vec3 t, vec3 n, mat3 r) {
    Vertex result;

    result.v = v;
    result.t = normalize(r * t);
    result.n = normalize(r * n);

    return result;
}
#endif

#ifdef TYPE_SKINNED
Vertex skinnedMesh(vec3 v, vec3 t, vec3 n, vec4 bones, vec4 weights) {
    Vertex result;
    result.v = vec3(0.0);
    result.t = vec3(0.0);
    result.n = vec3(0.0);

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

            mat3 m33 = mat3(m44[0].xyz,
                            m44[1].xyz,
                            m44[2].xyz);

            finalVector += (m44 * vec4(v, 1.0)) * weights.x;
            result.n += m33 * n * weights.x;
            result.t += m33 * t * weights.x;

            bones = bones.yzwx;
            weights = weights.yzwx;
        }
    }
    result.v = finalVector.xyz / finalVector.w;

    return result;
}
#endif

#ifdef TYPE_BILLBOARD
Vertex billboard(vec3 v, vec3 t, vec3 n, vec4 posRot, vec4 sizeDist) {
    Vertex result;
    result.t = t;
    result.n = n;

    float angle = posRot.w;  // rotation
    float x = cos(angle) * v.x + sin(angle) * v.y;
    float y = sin(angle) * v.x - cos(angle) * v.y;

    vec3 target = g.cameraTarget.xyz;
    if(g.cameraProjection[2].w < 0.0) {
        target = posRot.xyz;
    }
    vec3 normal = normalize(g.cameraPosition.xyz - target);
    vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), normal));
    vec3 up = normalize(cross(normal, right));

    result.v = (up * x + right * y) * sizeDist.xyz + posRot.xyz ;

    return result;
}
#endif

void main(void) {
#ifdef INSTANCING
    mat4 model = instanceMatrix;
#else
    mat4 model = l.model;
#endif
    mat4 mv = g.view * model;
    mat3 rot = mat3(model);

    vec3 camera = vec3(g.view[0].w,
                       g.view[1].w,
                       g.view[2].w);

#pragma vertex

#ifdef TYPE_STATIC
    Vertex vert = staticMesh(vertex, tangent, normal, rot);
    vert.v += WorldPositionOffset;
    _vertex = g.projection * (mv * vec4(vert.v, 1.0));
    _view = normalize((model * vec4(vert.v, 1.0)).xyz - g.cameraPosition.xyz);
#endif

#ifdef TYPE_BILLBOARD
    Vertex vert = billboard( vertex, tangent, normal, particlePosRot, particleSizeDist );
    vert.v += WorldPositionOffset;
    _vertex = g.projection * (mv * vec4(vert.v, 1.0));
    _view = normalize((model * vec4(vert.v, 1.0)).xyz - camera);
#endif

#ifdef TYPE_SKINNED
    Vertex vert = skinnedMesh(vertex, tangent, normal, bones, weights);
    vert.v += WorldPositionOffset;
    _vertex = g.projection * (g.view * vec4(vert.v, 1.0));
    _view = normalize(vert.v - g.cameraPosition.xyz);
#endif

    gl_Position = _vertex;

    _n     = vert.n;
    _t     = vert.t;
    _b     = cross ( _t, _n );
    _color = color;
    _uv0   = uv0;
}
