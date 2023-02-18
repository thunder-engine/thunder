<Shader>
    <Properties>
        <Property name="depthMap" type="texture2D" binding="0" target="true"/>
    </Properties>
    <Fragment>
<![CDATA[	
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D depthMap;

layout(location = 0) in vec4 _vertex;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec4 rgb;

void main(void) {
    vec2 proj = ((_vertex.xyz / _vertex.w) * 0.5 + 0.5).xy;

    float depth = getLinearDepth(texture(depthMap, proj).x, g.cameraPosition.w, g.cameraTarget.w);
    rgb = (depth >= _vertex.z) ? _color : vec4(_color.xyz, _color.w * 0.25);
}
]]>
    </Fragment>
    <Pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true" wireFrame="true"/>
</Shader>
