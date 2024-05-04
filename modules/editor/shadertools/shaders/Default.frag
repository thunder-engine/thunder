#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) out vec4 rgb;

void main(void) {
#pragma instance

#ifdef VISIBILITY_BUFFER
    color = vec4(objectId);
    return;
#endif

    rgb = vec4(1.0);
}
