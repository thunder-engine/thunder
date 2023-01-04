<Shader>
    <Properties>
        <Property name="showAlpha" type="float"/>
        <Property name="texture0" type="texture2D" binding="1" target="true"/>
    </Properties>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform Uniforms {
    float showAlpha;
} uni;

layout(binding = UNIFORM + 1) uniform sampler2D texture0;

layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 color;

void main() {
    color = vec4(texture(texture0, _uv0).xyz, 1.0);
}
]]>
    </Fragment>
    <Pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>