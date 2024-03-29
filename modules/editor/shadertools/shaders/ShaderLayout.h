#define GLOBAL     0
#define LOCAL      1
#define UNIFORM    4

layout(set = 1, binding = GLOBAL) uniform Global {
    mat4 view;
    mat4 projection;
    mat4 cameraView;
    mat4 cameraProjection;
    mat4 cameraProjectionInv;
    mat4 cameraScreenToWorld;
    mat4 cameraWorldToScreen;

    vec4 cameraPosition;
    vec4 cameraTarget;
    vec4 cameraScreen;
    vec4 shadowPageSize;

    float clip;
    float time;
    float deltaTime;
    float padding[13];
} g;

layout(binding = LOCAL) uniform Local {
    mat4 model;

    vec4 color;

    vec4 objectId;

    vec4 materialId;
} l;

layout(binding = LOCAL + 1) uniform sampler2D radianceMap;
