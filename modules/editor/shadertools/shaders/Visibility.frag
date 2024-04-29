#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 7) flat in int _instanceOffset;

layout(location = 0) out vec4 gbuffer0;

void main(void) {
#pragma instance

    gbuffer0 = vec4(objectId);
}
