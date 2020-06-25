#pragma version

#pragma flags

#include "Common.vert"

layout(location = 0) uniform mat4 t_model;
layout(location = 1) uniform mat4 t_view;
layout(location = 2) uniform mat4 t_projection;

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec4 color;
layout(location = 4) in vec2 uv0;

#ifdef INSTANCING
    layout(location = 8) in mat4 instanceMatrix;
#else
    #ifdef TYPE_BILLBOARD
    layout(location = 5) in vec4 particlePosRot;
    layout(location = 6) in vec4 particleSizeDist;
    layout(location = 7) in vec4 particleRes1;
    layout(location = 8) in vec4 particleRes2;
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

struct Vertex {
    vec3    v;
    vec3    t;
    vec3    n;
};

#pragma material

#ifdef TYPE_STATIC
Vertex staticMesh(vec3 v, vec3 t, vec3 n, mat3 r) {
    Vertex result;

    result.v = v;
    result.t = r * t;
    result.n = r * n;

    return result;
}
#endif

#ifdef TYPE_SKINNED
Vertex skinnedMesh(vec3 v, vec3 t, vec3 n, vec4 bones, vec4 weights) {
    Vertex result;
    result.v = vec3( 0.0 );
    result.t = vec3( 0.0 );
    result.n = vec3( 0.0 );

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

            vec4 tv = m44 * vec4(v, 1.0);
            result.v += (tv.xyz / tv.w) * weights.x;
            result.n += m33 * n * weights.x;
            result.t += m33 * t * weights.x;

            bones = bones.yzwx;
            weights = weights.yzwx;
        }
    }

    return result;
}
#endif

#ifdef TYPE_BILLBOARD
Vertex billboard(vec3 v, vec3 t, vec3 n, vec4 posRot, vec4 sizeDist) {
    Vertex result;
    result.t = t;
    result.n = n;

    float angle = posRot.w;  // rotation
    float x     = cos( angle ) * ( v.x ) + sin( angle ) * ( v.y );
    float y     = sin( angle ) * ( v.x ) - cos( angle ) * ( v.y );

    vec3 normal = normalize( camera.position.xyz - posRot.xyz );
    vec3 right  = normalize( cross( vec3(0.0, 1.0, 0.0), normal ) );
    vec3 up     = normalize( cross( normal, right ) );

    result.v    = (up * x + right * y) * sizeDist.xyz + posRot.xyz ;

    return result;
}
#endif

void main(void) {
#ifdef INSTANCING
    mat4 model  = instanceMatrix;
#else
    mat4 model  = t_model;
#endif
    mat4 mv     = t_view * model;
    mat3 rot    = mat3( model );

    vec3 camera = vec3( t_view[0].w,
                        t_view[1].w,
                        t_view[2].w );

#ifdef TYPE_STATIC
    Vertex vert = staticMesh( vertex, tangent, normal, rot );

    _vertex = t_projection * (mv * vec4(vert.v, 1.0));
    _view = ( model * vec4(vert.v, 1.0) ).xyz - camera;
#endif

#ifdef TYPE_BILLBOARD
    Vertex vert = billboard( vertex, tangent, normal, particlePosRot, particleSizeDist );

    _vertex = t_projection * (mv * vec4(vert.v, 1.0));
    _view = ( model * vec4(vert.v, 1.0) ).xyz - camera;
#endif

#ifdef TYPE_SKINNED
    Vertex vert = skinnedMesh( vertex, tangent, normal, bones, weights );

    _vertex = t_projection * (t_view * vec4(vert.v, 1.0));
    _view = vert.v - camera;
#endif

    gl_Position = _vertex;

    _n     = vert.n;
    _t     = vert.t;
    _b     = cross ( _t, _n );
    _color = color;
    _uv0   = uv0;
}
