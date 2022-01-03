#version 450 core

#include "ShaderLayout.h"

layout(std140, binding = UNIFORM) uniform Uniforms {
    vec2 direction;
    vec2 size;
    vec4 curve[8];
    int steps;
} uni;

layout(binding = UNIFORM + 1) uniform sampler2D rgbMap;

layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 rgb;

void main (void) {
    vec4 sum = texture(rgbMap, _uv0) * uni.curve[0].x;
    for(int i = 1; i < uni.steps; i++) {
        vec2 offset = vec2(float(i)) * uni.size * uni.direction;
        int r = i / 4;
        int b = int(mod(4, i));
        sum += texture(rgbMap, _uv0 - offset) * uni.curve[r][b];
        sum += texture(rgbMap, _uv0 + offset) * uni.curve[r][b];
    }
    rgb = sum;
}
