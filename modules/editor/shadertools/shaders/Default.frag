#version 450 core

#pragma flags

#define NO_INSTANCE
#include "ShaderLayout.h"

layout(location = 0) out vec4 rgb;

void main(void) {
#pragma objectId

#ifdef VISIBILITY_BUFFER
    rgb = _objectId;
    return;
#endif

    rgb = vec4(1.0);
}
