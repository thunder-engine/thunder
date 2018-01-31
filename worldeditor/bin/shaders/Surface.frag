#pragma version

#pragma flags

#include ".embedded/Common.vert"
#include ".embedded/BRDF.frag"

layout(location = 0) in vec3 _xyz;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec2 _uv1;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;
layout(location = 6) in vec4 _weights;

layout(location = 7) in vec3 _view;
layout(location = 8) in vec3 _proj;

uniform float       _clip;
uniform float       _time;

#pragma material

out vec4 gbuffer1;
out vec4 gbuffer2;
out vec4 gbuffer3;
out vec4 gbuffer4;

void simpleMode(Params params) {
    float alpha = getOpacity ( params );
#ifdef BLEND_OPAQUE
    if(_clip >= alpha) {
        discard;
    }
#endif
    gbuffer1    = transform.color;
}

void depthMode(Params params) {
    float depth = gl_FragCoord.z;
    float dx    = dFdx(depth);
    float dy    = dFdy(depth);
    float msqr  = depth * depth + 0.25 * (dx * dx + dy * dy);

    gbuffer1    = vec4(depth, msqr, 0.0, 0.0);
}

void passMode(Params params) {
    vec3 albd   = getDiffuse ( params ) * transform.color.xyz;
    vec3 emit   = getEmissive( params ) * transform.color.xyz;
    float alpha = getOpacity ( params ) * transform.color.w;
#ifdef BLEND_OPAQUE
    if(_clip >= alpha) {
        discard;
    }
    vec3 norm   = vec3(1.0);
    vec3 matv   = vec3(1.0, 1.0, getMetallic( params ));
    float rough = 0.0;
    float model = 0.0;
    #ifdef MODEL_LIT
    model       = 0.33;
    norm        = 0.5 * params.normals + vec3( 0.5 );
    rough       = max(0.08, getRoughness( params ));
    emit        = emit + albd * light.ambient;
    #endif
    gbuffer1    = vec4( norm, model );
    gbuffer2    = vec4( albd, rough );
    gbuffer3    = vec4( matv, rough );
    gbuffer4    = vec4( emit, 0.0   );
#else
    gbuffer1    = vec4( emit, alpha );
#endif
}

void main(void) {
    params.uv       = _uv0;
    params.project  = _proj;
    params.reflect  = vec3(0.0);
    params.weights  = _weights;
    params.normals  = _n;
    params.time     = _time;
#ifdef TANGENT
    params.normals  = 2.0 * getNormal( params ) - vec3( 1.0 );
    params.normals  = normalize( params.normals.x * _t + params.normals.y * _b + params.normals.z * _n );
#endif
    params.reflect  = reflect( _view, params.normals );
#ifdef SIMPLE
    #ifdef DEPTH
    depthMode(params);
    #else
    simpleMode(params);
    #endif
#else
    passMode(params);
#endif
}
