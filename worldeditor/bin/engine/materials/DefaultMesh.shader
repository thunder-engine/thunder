<shader version="11">
    <properties/>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 7) flat in int _instanceOffset;

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;
layout(location = 3) out vec4 gbuffer3;

void main() {
#pragma instance

    vec3 emissive = vec3(0.0f) * _color.xyz;
    vec3 albedo = vec3(1.0f) * _color.xyz;
    float roughness = 0.9f;
    float metallic = 0.0f;

    float model = 0.333f;

#ifdef VISIBILITY_BUFFER
    gbuffer0 = objectId;
    return;
#endif

    gbuffer0 = vec4(emissive, 0.0f);
    gbuffer1 = vec4(_n * 0.5f + 0.5f, model);
    gbuffer2 = vec4(albedo, model);
    gbuffer3 = vec4(roughness, 0.0f, metallic, 1.0f);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Lit" type="Surface" twoSided="false">
        <depth comp="Less" write="true" test="true"/>
    </pass>
</shader>
