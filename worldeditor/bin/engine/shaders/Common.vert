uniform struct Transform {
    vec4    color;
    mat4    view;
    mat4    model;
    mat4    projection;
    mat4    mv;
} transform;

uniform struct Camera {
    vec4    position;
    vec4    target;
    vec4    screen;
} camera;

uniform struct Light {
    mat4    mvpi;
    mat4    matrix[4];
    vec4    tiles[4];
    vec3    lod;
    vec3    pos;
    vec3    dir;
    vec3    color;
    float   bias;
    float   ambient;
    float   brightness;
    float   radius;
    float   shadows;
} light;

struct Params {
    vec2    uv;
    vec3    project;
    vec3    reflect;
    vec4    color;
    vec3    normal;
    float   time;
} params;

float sqr(float v) {
    return v * v;
}

float getLinearDepth (float value, float n, float f) {
    return n * f / ( f - value * ( f - n ) );
}

vec4 getWorld(mat4 mvpi, vec2 uv, float depth) {
    return mvpi * vec4( 2.0 * uv - 1.0, 2.0 * depth - 1.0, 1.0 );
}

float getAttenuation(float d, float r) {
    float offs0     = 1.0;
    float offs1     = 1.0 / ( 1.0 + r );
    float scale     = 0.5 / ( offs0 - offs1 );

    return scale * ( 1.0 / ( 1.0 + d ) - offs1 );
}

float luminanceApprox( vec3 color ) {
    return dot( color, vec3( 0.3, 0.6, 0.1 ) );
}

float linstep(float l, float h, float v) {
    return clamp((v-l)/(h-l), 0.0, 1.0);
}

float getShadowSample(sampler2D map, vec2 coord, float t) {
    return step(t, texture(map, coord).x);
}

float getShadowSampleLinear(sampler2D map, vec2 coord, float t) {
    vec2 pos    = coord / camera.screen.xy + vec2(0.5);
    vec2 frac   = fract(pos);
    vec2 start  = (pos - frac) * camera.screen.xy;

    float bl    = getShadowSample(map, start, t);
    float br    = getShadowSample(map, start + vec2(camera.screen.x, 0.0), t);
    float tl    = getShadowSample(map, start + vec2(0.0, camera.screen.y), t);
    float tr    = getShadowSample(map, start + camera.screen.xy, t);

    float a     = mix(bl, tl, frac.y);
    float b     = mix(br, tr, frac.y);

    return mix(a, b, frac.x);
}

float getShadowSamplePCF(sampler2D map, vec2 coord, float t) {
    const float NUM_SAMPLES     = 4.0;
    const float SAMPLES_START   = (NUM_SAMPLES - 1.0) * 0.5;

    float result    = 0.0;
    for(float y = -SAMPLES_START; y <= SAMPLES_START; y += 1.0) {
        for(float x = -SAMPLES_START; x <= SAMPLES_START; x += 1.0) {
            result += getShadowSampleLinear(map, coord + vec2(x, y) * camera.screen.xy, t);
        }
    }
    return result / (NUM_SAMPLES * NUM_SAMPLES);
}

float getShadowVarianceSample(sampler2D map, vec2 coord, float t) {
    vec2 m  = texture(map, coord).xy;

    float p = step(t, m.x);
    float v = max(m.y - m.x * m.x, 0.000002); // 0.000002 = Min variance

    float d = t - m.x;
    float pm = linstep(0.2, 1.0, v / (v + d * d));

    return clamp(max(p, pm), 0.0, 1.0);
}

float getShadow(sampler2D map, vec2 coord, float t) {
    return getShadowSample(map, coord, t);
}
