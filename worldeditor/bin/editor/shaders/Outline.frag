#pragma version

#include "Common.vert"

layout(location = 40) uniform sampler2D rgbMap;
layout(location = 41) uniform sampler2D outlineMap;
layout(location = 42) uniform vec4 color;
layout(location = 43) uniform float width;

layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 rgb;

void main (void) {
    vec2 up    = vec2(0.0, camera.screen.y) * width;
    vec2 right = vec2(camera.screen.x, 0.0) * width;

    float c = dot(texture(outlineMap, _uv0), vec4(1));
    float t = dot(texture(outlineMap, _uv0 - up), vec4(1));
    float l = dot(texture(outlineMap, _uv0 - right), vec4(1));
    float r = dot(texture(outlineMap, _uv0 + right), vec4(1));
    float b = dot(texture(outlineMap, _uv0 + up), vec4(1));

    vec2 n = vec2(-(t - b), (r - l));
    float v = (length( n ) > 0.1) ? 1.0 : 0.0;

    rgb = mix(texture(rgbMap, _uv0), color, v);
}
