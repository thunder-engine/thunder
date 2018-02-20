#pragma version

#include ".embedded/Common.vert"

uniform sampler2D   rgbMap;
uniform float       threshold; // half

layout(location = 0) in vec2 _uv;

out vec4 rgb; // half4

void main(void) {
    rgb = texture2D ( rgbMap, _uv + camera.screen.xy * vec2( 0.5, 0.5)) + // half2
          texture2D ( rgbMap, _uv + camera.screen.xy * vec2(-0.5,-0.5)) + // half2
          texture2D ( rgbMap, _uv + camera.screen.xy * vec2( 0.5,-0.5)) + // half2
          texture2D ( rgbMap, _uv + camera.screen.xy * vec2(-0.5, 0.5));  // half2

    rgb = max(rgb * 0.25 - threshold, 0.0);
}
