<shader version="11">
    <properties>
        <property binding="0" type="texture2d" name="rgbMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

#define FXAA_SPAN_MAX 16.0
#define FXAA_REDUCE_MUL   (1.0/FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_SUBPIX_SHIFT (1.0/4.0)

layout(binding = UNIFORM) uniform sampler2D rgbMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 0) out vec4 rgb;

#include "Functions.h"

void main (void) {
    vec2 nw = _uv0 + vec2(-g.cameraScreen.z,-g.cameraScreen.w);
    vec2 ne = _uv0 + vec2( g.cameraScreen.z,-g.cameraScreen.w);
    vec2 sw = _uv0 + vec2(-g.cameraScreen.z, g.cameraScreen.w);
    vec2 se = _uv0 + vec2( g.cameraScreen.z, g.cameraScreen.w);
        
    vec3 rgbNW = texture( rgbMap, nw ).xyz;
    vec3 rgbNE = texture( rgbMap, ne ).xyz;
    vec3 rgbSW = texture( rgbMap, sw ).xyz;
    vec3 rgbSE = texture( rgbMap, se ).xyz;
    vec3 rgbM  = texture( rgbMap, _uv0 ).xyz;

    float lumaNW = luminanceApprox(rgbNW);
    float lumaNE = luminanceApprox(rgbNE);
    float lumaSW = luminanceApprox(rgbSW);
    float lumaSE = luminanceApprox(rgbSE);
    float lumaM  = luminanceApprox(rgbM);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max( (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN );
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) * g.cameraScreen.zw;

    vec3 rgbA = (1.0/2.0) * (
        texture(rgbMap, _uv0 + dir * (1.0/3.0 - 0.5)).xyz +
        texture(rgbMap, _uv0 + dir * (2.0/3.0 - 0.5)).xyz);

    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        texture(rgbMap, _uv0 + dir * (0.0/3.0 - 0.5)).xyz +
        texture(rgbMap, _uv0 + dir * (3.0/3.0 - 0.5)).xyz);

    float lumaB = luminanceApprox(rgbB);

    if((lumaB < lumaMin) || (lumaB > lumaMax)) {
        rgb = vec4(rgbA, 1.0f);
    } else {
        rgb = vec4(rgbB, 1.0f);
    }
}

]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true"/>
</shader>
