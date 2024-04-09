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

layout(binding = LOCAL) uniform sampler2D instanceMap;

layout(binding = LOCAL + 1) uniform sampler2D radianceMap;

mat4 getModelMatrix() {
    float width = 1.0 / 512.0;
    int x = gl_InstanceID * 8;
    int y = 0;

    vec4 m1 = texture(instanceMap, vec2(x,     y) * width);
    vec4 m2 = texture(instanceMap, vec2(x + 1, y) * width);
    vec4 m3 = texture(instanceMap, vec2(x + 2, y) * width);
    vec4 m4 = texture(instanceMap, vec2(x + 3, y) * width);

    return mat4(m1, m2, m3, m4);
}
