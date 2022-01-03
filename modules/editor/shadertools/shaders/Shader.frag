#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 3) uniform vec4  t_color;
layout(location = 4) uniform float _clip;
layout(location = 5) uniform float _time;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec2 _uv1;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;
layout(location = 6) in vec4 _color;

layout(location = 7) in vec3 _view;

layout(location = 0) out vec4 gbuffer1;
layout(location = 1) out vec4 gbuffer2;
layout(location = 2) out vec4 gbuffer3;
layout(location = 3) out vec4 gbuffer4;

#pragma fragment

void simpleMode() {
    float alpha = getOpacity ( params );
    if(alpha <= _clip) {
        discard;
    }
    gbuffer1    = t_color;
}

void passMode() {
    vec3 albd   = getDiffuse ( params ) * t_color.xyz;
    vec3 emit   = getEmissive( params ) * t_color.xyz;
    float alpha = getOpacity ( params ) * t_color.w;
    float spec  = 1.0;
#ifdef BLEND_OPAQUE
    if(_clip >= alpha) {
        discard;
    }
    vec3 norm   = vec3(1.0);
    vec3 matv   = vec3(0.0, 0.0, getMetallic( params ));
    float model = 0.0;
    #ifdef MODEL_LIT
    model       = 0.34;
    norm        = params.normal * 0.5 + 0.5;
    emit        = emit + albd * light.ambient;
    matv.x      = max(0.01, getRoughness( params ));
    #endif
    gbuffer1    = vec4( norm, model );
    gbuffer2    = vec4( albd, 1.0   );
    gbuffer3    = vec4( matv, spec  );
    gbuffer4    = vec4( emit, 0.0   );
#else
    gbuffer1    = vec4( emit, alpha );
#endif
}

void lightMode() {
    gbuffer1    = vec4( getEmissive( params ), 1.0 );
}

void main(void) {
    params.reflect  = vec3(0.0);
    params.normal   = _n;

#ifdef SIMPLE
    simpleMode();
#elif LIGHT
    lightMode();
#else
    params.normal   = getNormal( params ) * 2.0 - 1.0;
    params.normal   = normalize( params.normal.x * _t + params.normal.y * _b + params.normal.z * _n );
    params.reflect  = reflect( _view, params.normal );

    passMode();
#endif
}
