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
    vec4 result = mat * vec4(uv * 2.0f - 1.0f, depth * 2.0f - 1.0f, 1.0f);
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
    return dot(rgb, vec3(0.3, 0.6, 0.1));
}

float linstep(float l, float h, float v) {
    return clamp((v - l) / (h - l), 0.0f, 1.0f);
}

// Shadow map functions
float getShadowSample(sampler2D map, vec2 coord, float t) {
    return step(t, texture(map, coord).x);
}

float getShadowSampleLinear(sampler2D map, vec2 coord, float t) {
    vec2 pos = coord / g.shadowPageSize.xy + vec2(0.5);
    vec2 frac = fract(pos);
    vec2 start = (pos - frac) * g.shadowPageSize.xy;

    float bl = getShadowSample(map, start, t);
    float br = getShadowSample(map, start + vec2(g.shadowPageSize.x, 0.0f), t);
    float tl = getShadowSample(map, start + vec2(0.0f, g.shadowPageSize.y), t);
    float tr = getShadowSample(map, start + g.shadowPageSize.xy, t);

    float a = mix(bl, tl, frac.y);
    float b = mix(br, tr, frac.y);

    return mix(a, b, frac.x);
}

float getShadowSamplePCF(sampler2D map, vec2 coord, float t) {
    const float NUM_SAMPLES = 4.0f;
    const float SAMPLES_START = (NUM_SAMPLES - 1.0f) * 0.5;

    float result = 0.0f;
    for(float y = -SAMPLES_START; y <= SAMPLES_START; y += 1.0f) {
        for(float x = -SAMPLES_START; x <= SAMPLES_START; x += 1.0f) {
            result += getShadowSampleLinear(map, coord + vec2(x, y) * g.shadowPageSize.xy, t);
        }
    }
    return result / (NUM_SAMPLES * NUM_SAMPLES);
}

float getShadowVarianceSample(sampler2D map, vec2 coord, float t) {
    vec2 m  = texture(map, coord).xy;

    float p = step(t, m.x);
    float v = max(m.y - m.x * m.x, 0.000002f); // 0.0f00002 = Min variance

    float d = t - m.x;
    float pm = linstep(0.2, 1.0f, v / (v + d * d));

    return max(p, pm);
}

float getShadow(sampler2D map, vec2 coord, float t) {
    return clamp(getShadowSamplePCF(map, coord, t), 0.0f, 1.0f);
}
