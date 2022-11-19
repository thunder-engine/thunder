<Shader>
    <Properties>
        <Property name="ssaoMap" type="texture2D" binding="5" target="true"/>
        <Property name="normalsMap" type="texture2D" binding="6" target="true"/>
    </Properties>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM + 1) uniform sampler2D ssaoMap;
layout(binding = UNIFORM + 2) uniform sampler2D normalsMap;

layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 color;

void main() {
    // Light model LIT
    if(texture(normalsMap, _uv0).w > 0.33) {
        color = vec4(0, 0, 0, 1.0 - texture(ssaoMap, _uv0).x);
    } else {
        color = vec4(0, 0, 0, 0);
    }
}
]]>
    </Fragment>
    <Pass type="PostProcess" blendMode="Translucent" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>
