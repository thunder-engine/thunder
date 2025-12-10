#define GLOBAL     0
#define LOCAL      1
#define UNIFORM    4

layout(set = 1, binding = GLOBAL) uniform Global {
    mat4 view;
    mat4 projection;
    mat4 cameraWorldToScreen;
    mat4 cameraAndParams;
} g;

mat4 viewMatrix() {
    return g.view;
}

mat4 projectionMatrix() {
    return g.projection;
}

mat4 projectionMatrixInv() {
    return inverse(projectionMatrix());
}

mat4 cameraWorldToScreen() {
    return g.cameraWorldToScreen;
}

mat4 cameraScreenToWorld() {
    return inverse(cameraWorldToScreen());
}

bool isOrtho() {
    return g.projection[2].w < 0.0f;
}

float nearClipPlane() {
    return g.cameraAndParams[1].x;
}

float farClipPlane() {
    return g.cameraAndParams[1].y;
}

float time() {
    return g.cameraAndParams[2].y;
}

float deltaTime() {
    return g.cameraAndParams[2].z;
}

float shadowPageSize() {
    return g.cameraAndParams[2].x;
}

vec2 screenSizeNorm() {
    return g.cameraAndParams[1].zw;
}

vec2 screenSize() {
    return 1.0f / screenSizeNorm();
}

vec3 cameraPosition() {
    return g.cameraAndParams[0].xyz;
}

vec3 cameraDirection() {
    return mat3(viewMatrix()) * vec3(0.0f, 0.0f, 1.0f);
}

#ifndef NO_INSTANCE

#ifdef USE_SSBO
layout(std430, binding = LOCAL) buffer InstanceData {
    vec4 data[];
} instance;
#else
layout(std140, binding = LOCAL) uniform InstanceData {
    vec4 data[4096];
} instance;
#endif

mat4 modelMatrix() {
    return mat4(vec4(instance.data[_instanceOffset + 0].xyz, 0.0f),
                vec4(instance.data[_instanceOffset + 1].xyz, 0.0f),
                vec4(instance.data[_instanceOffset + 2].xyz, 0.0f),
                vec4(instance.data[_instanceOffset + 3].xyz, 1.0f));
}

mat4 modelViewMatrix() {
    return viewMatrix() * modelMatrix();
}

#endif
