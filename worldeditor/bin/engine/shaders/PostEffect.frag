#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D rgbMap;

layout(location = 1) in vec2 _uv0;

out vec4 rgb;

void main (void) {
    rgb = texture2D( rgbMap, _uv0 );
}
