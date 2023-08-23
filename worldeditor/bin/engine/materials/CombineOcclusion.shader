<shader>
    <properties>
        <property name="ssaoMap" type="texture2D" binding="1" target="true"/>
        <property name="normalsMap" type="texture2D" binding="2" target="true"/>
    </properties>
    <fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM + 1) uniform sampler2D ssaoMap;
layout(binding = UNIFORM + 2) uniform sampler2D normalsMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

layout(location = 0) out vec4 color;

void main() {
    // Light model LIT
    if(texture(normalsMap, _uv0).w > 0.0) {
        color = vec4(0, 0, 0, 1.0 - texture(ssaoMap, _uv0).x);
    } else {
        color = vec4(0, 0, 0, 0);
    }
}
]]>
    </fragment>
    <pass type="PostProcess" blendMode="Translucent" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</shader>
