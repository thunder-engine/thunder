<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="rgbMap" binding="0" type="texture2d" target="true" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

#define FXAA_SPAN_MAX 8.0
#define FXAA_REDUCE_MUL   (1.0/FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN   (1.0/128.0)

layout(binding = UNIFORM) uniform sampler2D rgbMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 rgb;

#include "Functions.h"

void main (void) {
    vec2 nw = _uv0 + vec2(-screenSizeNorm().x,-screenSizeNorm().y);
    vec2 ne = _uv0 + vec2( screenSizeNorm().x,-screenSizeNorm().y);
    vec2 sw = _uv0 + vec2(-screenSizeNorm().x, screenSizeNorm().y);
    vec2 se = _uv0 + vec2( screenSizeNorm().x, screenSizeNorm().y);
        
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
          dir * rcpDirMin)) * screenSizeNorm();

    vec3 rgbA = 0.5f * (
        texture(rgbMap, _uv0 + dir * (1.0f / 3.0f - 0.5f)).xyz +
        texture(rgbMap, _uv0 + dir * (2.0f / 3.0f - 0.5f)).xyz);

    vec3 rgbB = rgbA * 0.5f + 0.25f * (
        texture(rgbMap, _uv0 + dir * -0.5f).xyz +
        texture(rgbMap, _uv0 + dir *  0.5f).xyz);

    float lumaB = luminanceApprox(rgbB);

    if((lumaB < lumaMin) || (lumaB > lumaMax)) {
        rgb = vec4(rgbA, 1.0f);
    } else {
        rgb = vec4(rgbB, 1.0f);
    }
}

]]></fragment>
    <pass type="PostProcess" twoSided="true" lightModel="Unlit" wireFrame="false" />
</shader>
