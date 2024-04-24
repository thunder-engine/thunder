const int structSize = 8;

mat4 getModelMatrix() {
    return mat4(instance.data[gl_InstanceIndex * structSize + 0],
                instance.data[gl_InstanceIndex * structSize + 1],
                instance.data[gl_InstanceIndex * structSize + 2],
                instance.data[gl_InstanceIndex * structSize + 3]);
}

vec4 getLocalColor() {
    return vec4(instance.data[gl_InstanceIndex * structSize + 4]);
}
