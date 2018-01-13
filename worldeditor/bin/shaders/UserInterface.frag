#pragma version

#include ".embedded/Common.vert"

uniform sampler2D texture;

in vec2 _uv;

#pragma material

out vec4 rgb;

void main(void) {
    rgb = vec4(vec3(1.0), texture2D(texture, _uv).a);
}
