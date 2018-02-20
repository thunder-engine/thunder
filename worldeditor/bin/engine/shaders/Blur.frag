#pragma version

uniform sampler2D   rgbMap;

uniform vec2 direction; // half2
uniform vec2 size; // half2
uniform int steps;
uniform float curve[32];

layout(location = 0) in vec2 _uv;

out vec4 rgb; // half4

void main (void) {
    vec4 sum    = texture2D( rgbMap, _uv ) * curve[0];
    for(int i = 1; i < steps; i++) {
        vec2 offset = float(i) * size * direction; // half2 

        sum += texture2D( rgbMap, _uv - offset ) * curve[i];
        sum += texture2D( rgbMap, _uv + offset ) * curve[i];
    }

    rgb = sum; // / steps
}
