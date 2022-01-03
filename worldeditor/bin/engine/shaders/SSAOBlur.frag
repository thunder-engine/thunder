#version 450 core

#define MAX_SAMPLE_COUNT 16

#include "ShaderLayout.h"

layout(location = 50) uniform sampler2D ssaoSample;

layout(location = 1) in vec2 _uv0;

layout(location = 0) out float color;

const int blurRange = 2;

void main() {
    int n = 0;
    float result = 0.0;
    for(int x = -blurRange; x < blurRange; x++) {
        for(int y = -blurRange; y < blurRange; y++) {
            vec2 offset = vec2(float(x), float(y)) * camera.screen.xy;
            result += texture(ssaoSample, _uv0 + offset).r;
            n++;
        }
    }
    color = result / (float(n));
}
