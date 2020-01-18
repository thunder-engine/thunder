#pragma version

#include "Common.vert"

layout(location = 40) uniform sampler2D rgbMap;
layout(location = 41) uniform float threshold;

layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 rgb;

void main(void) {
    rgb = texture ( rgbMap, _uv0 + camera.screen.xy * vec2( 0.5, 0.5)) +
          texture ( rgbMap, _uv0 + camera.screen.xy * vec2(-0.5,-0.5)) +
          texture ( rgbMap, _uv0 + camera.screen.xy * vec2( 0.5,-0.5)) +
          texture ( rgbMap, _uv0 + camera.screen.xy * vec2(-0.5, 0.5));

    rgb = max(rgb * 0.25 - threshold, 0.0);
}
