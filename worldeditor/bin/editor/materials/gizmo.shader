<Shader>
    <Properties/>
    <Fragment>
<![CDATA[	
#version 450 core

#include "ShaderLayout.h"

layout(location = 0) out vec4 rgb;

void main(void) {
    rgb = l.color;
}
]]>
    </Fragment>
    <Pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>
