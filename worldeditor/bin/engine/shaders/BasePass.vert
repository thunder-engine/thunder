#pragma version

#pragma flags

#include "Common.vert"

#define MAX_BONES 60

layout(location = 0) uniform mat4 t_model;
layout(location = 1) uniform mat4 t_view;
layout(location = 2) uniform mat4 t_projection;

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec4 color;
layout(location = 4) in vec2 uv0;
#ifdef INSTANCING
    layout(location = 5) in mat4 instanceMatrix;
#else
    #ifdef TYPE_BILLBOARD
    layout(location = 5) in vec4 particlePosRot;
    layout(location = 6) in vec4 particleSizeDist;
    layout(location = 7) in vec4 particleRes1;
    layout(location = 8) in vec4 particleRes2;
    #else
    //layout(location = 4) in vec2 uv1;
    //layout(location = 5) in vec2 uv2;
    //layout(location = 6) in vec2 uv3;
    //layout(location = 8) in vec4 indices;
    //layout(location = 9) in vec4 weights;
    #endif
#endif
layout(location = 0) out vec4 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec2 _uv1;
layout(location = 3) out vec3 _n;
layout(location = 4) out vec3 _t;
layout(location = 5) out vec3 _b;
layout(location = 6) out vec4 _color;

layout(location = 7) out vec3 _view;

struct Vertex {
    vec3    v;
    vec3    t;
    vec3    n;
};

#pragma material

Vertex staticMesh(vec3 v, vec3 t, vec3 n, mat3 r) {
    Vertex result;

    result.v    = v;
    result.t    = r * t;
    result.n    = r * n;

    return result;
}

Vertex skinnedMesh(vec3 v, vec3 t, vec3 n, vec4 indices, vec4 weights) {
    Vertex result;
    result.v    = vec3( 0.0 );
    result.t    = vec3( 0.0 );
    result.n    = vec3( 0.0 );
/*
    for(int i = 0; int(v.w) > i ; i++) { // process bones
        mat4 m44    = bonesMatrix[int(indices.x)];

        mat3 m33    = mat3( m44[0].xyz,
                            m44[1].xyz,
                            m44[2].xyz );

        result.v    += m44 * vec4( v, 1.0 ) * weights.x;
        result.n    += m33 * n * weights.x;
        result.t    += m33 * t * weights.x;

        indices = indices.yzwx;
        weights = weights.yzwx;
    }

    result.v    = vec4( result.v.xyz, 1.0 );
*/
    return result;
}

Vertex axisAlignedBillboard(vec3 v, vec3 t, vec3 n, vec3 axis) {
    Vertex result;
    result.t    = t;
    result.n    = n;
/*
    vec3 look   = normalize( camera.position.xyz - position.xyz );
    vec3 right  = normalize( cross(axis, look) );

    mat4 m44    = mat4( vec4(right,         0.0),
                        vec4(axis,          0.0),
                        vec4(look,          0.0),
                        vec4(position.xyz,  1.0));

    result.v    = m44 * vec4( v.x * v.z, v.y * v.w, 0.0, 1.0 );
*/
    return result;
}

Vertex billboard(vec3 v, vec3 t, vec3 n, vec4 posRot, vec4 sizeDist) {
    Vertex result;
    result.t    = t;
    result.n    = n;

    float angle = posRot.w;  // rotation
    float x     = cos( angle ) * ( v.x ) + sin( angle ) * ( v.y );
    float y     = sin( angle ) * ( v.x ) - cos( angle ) * ( v.y );

    vec3 normal = normalize( camera.position.xyz - posRot.xyz );
    vec3 right  = normalize( cross( vec3(0.0, 1.0, 0.0), normal ) );
    vec3 up     = normalize( cross( normal, right ) );

    result.v    = (up * x + right * y) * sizeDist.xyz + posRot.xyz ;

    return result;
}

void main(void) {
#ifdef INSTANCING
    mat4 model  = instanceMatrix;
#else
    mat4 model  = t_model;
#endif
    mat4 mv     = t_view * model;
    mat3 rot    = mat3( model );

#ifdef TYPE_STATIC
    Vertex vert = staticMesh( vertex.xyz, tangent, normal, rot );
#endif

#ifdef TYPE_SKINNED
    Vertex vert = skinnedMesh( vertex.xyz, tangent, normal, indices, weights );
#endif

#ifdef TYPE_BILLBOARD
    Vertex vert = billboard( vertex.xyz, tangent, normal, particlePosRot, particleSizeDist );
#endif

#ifdef TYPE_AXISALIGNED
    Vertex vert = axisAlignedBillboard( vertex.xyz, tangent, normal, axis );
#endif
    vec3 camera = vec3( t_view[0].w,
                        t_view[1].w,
                        t_view[2].w );

    _vertex     = t_projection * (mv * vec4(vert.v, 1.0));
    gl_Position = _vertex;

    _n     = vert.n;
    _t     = vert.t;
    _b     = cross ( _t, _n );
    _color = color;
    _uv0   = uv0;
#ifndef INSTANCING
    //_uv1 = uv1;
#endif
    _view  = ( model * vec4(vert.v, 1.0) ).xyz - camera;
}
