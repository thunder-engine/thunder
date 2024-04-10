const int structSize = 8;

ivec3 getTextureCoord(int offset) {
    ivec3 result;

    result.z = textureSize(instanceMap, 0).x;
    result.x = gl_InstanceIndex * structSize + offset;
    result.y = 0;

    return result;
}

mat4 getModelMatrix() {
    ivec3 offset = getTextureCoord(0);
    float width = 1.0 / offset.z;

    vec4 m1 = textureLod(instanceMap, vec2(offset.x,     offset.y) * width, 0);
    vec4 m2 = textureLod(instanceMap, vec2(offset.x + 1, offset.y) * width, 0);
    vec4 m3 = textureLod(instanceMap, vec2(offset.x + 2, offset.y) * width, 0);
    vec4 m4 = textureLod(instanceMap, vec2(offset.x + 3, offset.y) * width, 0);

    return mat4(m1, m2, m3, m4);
}

vec4 getLocalColor() {
    ivec3 offset = getTextureCoord(4);
    float width = 1.0 / offset.z;

    return textureLod(instanceMap, vec2(offset.xy) * width, 0);
}
