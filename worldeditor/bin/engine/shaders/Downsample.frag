#pragma version

#include "Common.vert"

layout(location = 50) uniform sampler2D rgbMap;
layout(location = 51) uniform float threshold;

layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 color;

void main(void) {
    color = texture(rgbMap, _uv0 + camera.screen.xy * vec2( 0.5, 0.5)) +
            texture(rgbMap, _uv0 + camera.screen.xy * vec2(-0.5,-0.5)) +
            texture(rgbMap, _uv0 + camera.screen.xy * vec2( 0.5,-0.5)) +
            texture(rgbMap, _uv0 + camera.screen.xy * vec2(-0.5, 0.5));

    color = max(color * 0.25 - threshold, 0.0);
    color = max(texture(rgbMap, _uv0) - 1.0, 0.0);
}
