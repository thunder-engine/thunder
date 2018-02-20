#pragma version

uniform sampler2D   rgbMap;

layout(location = 1) in vec2 _uv0;

out vec4 rgb;

void main (void) {
    vec4 color  = texture2D( rgbMap, _uv0 );
    rgb = color;
}
