<shader version="11">
    <properties>
        <property binding="0" type="texture2d" name="ssaoSample" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D ssaoSample;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 0) out float color;

const int blurRange = 2;

void main() {
    int n = 0;
    float result = 0.0;

    for(int x = -blurRange; x < blurRange; x++) {
        for(int y = -blurRange; y < blurRange; y++) {
            vec2 offset = vec2(float(x), float(y)) * g.cameraScreen.zw;
            result += texture(ssaoSample, _uv0 + offset).r;
            n++;
        }
    }
    color = result / (float(n));
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true"/>
</shader>
