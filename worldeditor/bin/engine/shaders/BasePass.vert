#pragma version

#pragma flags

#include ".embedded/Common.vert"

#define MAX_BONES 60

uniform mat4 bonesMatrix[MAX_BONES];
uniform vec4 position;
uniform vec3 axis;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv0;
layout(location = 2) in vec2 uv1;
layout(location = 3) in vec2 uv2;
layout(location = 4) in vec2 uv3;
layout(location = 5) in vec3 normal;
layout(location = 6) in vec3 tangent;
layout(location = 7) in vec4 color;
layout(location = 8) in vec4 indices;
layout(location = 9) in vec4 weights;

layout(location = 0) out vec3 _vertex;
layout(location = 1) out vec2 _uv0;
layout(location = 2) out vec2 _uv1;
layout(location = 3) out vec3 _n;
layout(location = 4) out vec3 _t;
layout(location = 5) out vec3 _b;
layout(location = 6) out vec4 _color;

layout(location = 7) out vec3 _view;
layout(location = 8) out vec3 _proj;

out gl_PerVertex {
    vec4  gl_Position;
};

struct Vertex {
    vec3    v;
    vec3    t;
    vec3    n;
    vec4    m;
};

#pragma material

Vertex staticMesh(vec3 v, vec3 t, vec3 n) {
    Vertex result;
    result.v    = v;
    result.t    = t;
    result.n    = n;
    result.m    = vec4( 0.0 );

    return result;
}

Vertex skinnedMesh(vec3 v, vec3 t, vec3 n, vec4 indices, vec4 weights) {
    Vertex result;
    result.v    = vec3( 0.0 );
    result.t    = vec3( 0.0 );
    result.n    = vec3( 0.0 );
    result.m    = vec4( 0.0 );
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

Vertex axisAlignedBillboard(vec3 v, vec3 t, vec3 n, vec4 position, vec3 axis) {
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
    result.m    = vec4( 0.0 );
*/
    return result;
}

Vertex billboard(vec3 v, vec3 t, vec3 n, vec4 position) {
    Vertex result;
    result.t    = t;
    result.n    = n;
/*
    float angle = position.w;  // w - Rotation
    // Billboard rotated coordinates in {0 0 0} matrix
    float x     = cos( angle ) * ( v.z ) + sin( angle ) * ( v.w );
    float y     = sin( angle ) * ( v.z ) - cos( angle ) * ( v.w );

    result.v    = vec4( position.xyz, 1.0 );
    result.m    = vec4( v.x * x, v.y * y, 0.0, 0.0 );
*/
    return result;
}

void main(void) {
#ifdef TYPE_STATIC
    Vertex vert = staticMesh( vertex, tangent, normal );
#endif

#ifdef TYPE_SKINNED
    Vertex vert = skinnedMesh( vertex, tangent, normal, indices, weights );
#endif

#ifdef TYPE_BILLBOARD
    Vertex vert = billboard( vertex, tangent, normal, position );
#endif

#ifdef TYPE_AXISALIGNED
    Vertex vert = axisAlignedBillboard( vertex, tangent, normal, position, axis );
#endif
    gl_Position = transform.projection * ( ( transform.mv * vec4(vert.v, 1.0) ) + vert.m );

    _vertex     = gl_Position.xyz;
    _n          = vert.n;
    _t          = vert.t;
    _b          = cross ( _t, _n );

    _color      = color;
    _uv0        = uv0;
    _uv1        = uv1;
    _proj       = 0.5 * ( gl_Position.xyz / gl_Position.w ) + 0.5;
    _view       = ( transform.model * vec4(vert.v, 1.0) ).xyz - camera.position.xyz;
}
