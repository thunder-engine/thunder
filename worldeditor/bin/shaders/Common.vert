uniform struct Transform {
    vec4    color;
    mat4    view;
    mat4    model;
    mat4    projection;
    mat4    mv;
    mat4    mvpi;
} transform;

uniform struct Camera {
    vec4    position;
    vec4    target;
    vec2    screen;
} camera;

uniform struct Light {
    mat4    matrix[4];
    vec3    pos;
    vec3    dir;
    vec3    color;
    vec3    lod;
    float   ambient;
    float   brightness;
    float   radius;
    float   shadows;
} light;

struct Params {
    vec2    uv;
    vec3    project;
    vec3    reflect;
    vec4    weights;
    vec3    normals;
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

float getAttenuation (float d, float r) {
    float offs0     = 1.0;
    float offs1     = 1.0 / ( 1.0 + r );
    float scale     = 0.5 / ( offs0 - offs1 );

    return scale * ( 1.0 / ( 1.0 + d ) - offs1 );
}

float luminanceApprox ( vec3 color ) {
    return dot( color, vec3( 0.3, 0.6, 0.1 ) );
}
