<Shader>
    <Properties/>
    <Fragment>
<![CDATA[	
#version 450 core

#include "ShaderLayout.h"

layout(location = 3) in vec3 _n;

layout(location = 0) out vec4 rgb;

void main(void) {
    vec3 emit = (vec3(0.4) + vec3(dot(_n, normalize(g.cameraTarget.xyz)))) * l.color.xyz;
    rgb = vec4(emit, l.color.w);
}
]]>		
    </Fragment>
    <Pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="true" depthWrite="true" twoSided="true"/>
</Shader>
