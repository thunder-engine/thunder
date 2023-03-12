<Shader>
    <Properties>
        <Property name="ssaoSample" type="texture2D" binding="1" target="true"/>
    </Properties>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM + 1) uniform sampler2D ssaoSample;

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
]]>
    </Fragment>
    <Pass type="PostProcess" blendMode="Opaque" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>
