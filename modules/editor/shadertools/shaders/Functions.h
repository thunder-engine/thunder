const float PI = 3.14159265358979323846;

float sqr(float v) {
    return v * v;
}

vec3 qtransform(vec4 q, vec3 v) {
    return v + 2.0f * cross(cross(v, q.xyz) + q.w * v, q.xyz);
}

float getLinearDepth(float value, float n, float f) {
    return n * f / (f - value * (f - n));
}

vec3 getWorld(mat4 mat, vec2 uv, float depth) {
    vec4 result = mat * vec4(uv * 2.0f - 1.0f, depth
#ifndef VULKAN
                             * 2.0f - 1.0f
#endif
                             , 1.0f);
    return result.xyz / result.w;
}

vec3 getView(mat4 mat, vec2 uv, float depth) {
    vec4 result = mat * vec4(uv * 2.0f - 1.0f, depth, 1.0f);
    return result.xyz / result.w;
}

vec2 cartesianToSpherical(vec3 dir) {
    float temp = atan(dir.z, dir.x);
    float phi = temp < 0.0f ? (temp + PI * 2.0f) : temp;
    float theta = acos(dir.y);

    return vec2(phi / (2.0 * PI), theta / PI);
}

float getAttenuation(float d, float r) {
    float offs0 = 1.0f;
    float offs1 = 1.0f / (1.0f + r);
    float scale = 0.5 / (offs0 - offs1);

    return scale * (1.0f / (1.0f + d) - offs1);
}

float luminanceApprox(vec3 rgb) {
    return dot(rgb, vec3(0.299, 0.587, 0.114));
}

float linstep(float l, float h, float v) {
    return clamp((v - l) / (h - l), 0.0f, 1.0f);
}
