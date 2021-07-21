#version 450 core

#include "ShaderLayout.h"

layout(binding = 4) uniform Uniforms {
    vec4 color;
    float width;
} uni;

layout(binding = 5) uniform sampler2D rgbMap;
layout(binding = 6) uniform sampler2D outlineMap;

layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 rgb;

void main (void) {
    vec2 up    = vec2(0.0, g.cameraScreen.y) * uni.width;
    vec2 right = vec2(g.cameraScreen.x, 0.0) * uni.width;

    float c = dot(texture(outlineMap, _uv0), vec4(1));
    float t = dot(texture(outlineMap, _uv0 - up), vec4(1));
    float l = dot(texture(outlineMap, _uv0 - right), vec4(1));
    float r = dot(texture(outlineMap, _uv0 + right), vec4(1));
    float b = dot(texture(outlineMap, _uv0 + up), vec4(1));

    vec2 n = vec2(-(t - b), (r - l));
    float v = (length( n ) > 0.1) ? 1.0 : 0.0;

    rgb = mix(texture(rgbMap, _uv0), uni.color, v);
}
