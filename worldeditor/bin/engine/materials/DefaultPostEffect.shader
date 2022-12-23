<Shader>
    <Properties>
        <Property name="color" type="vec4"/>
        <Property name="texture0" type="texture2D" binding="1" target="false"/>
    </Properties>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform Uniforms {
    vec4 color;
} uni;

layout(binding = UNIFORM + 1) uniform sampler2D texture0;

layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 color;

void main() {
    color = texture(texture0, _uv0) * l.color * uni.color;
}
]]>
    </Fragment>
    <Pass type="PostProcess" blendMode="Translucent" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>
