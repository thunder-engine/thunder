<shader>
    <properties>
        <property name="color0" type="vec4"/>
    </properties>
    <fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform Uniforms {
        vec4 color0;
} uni;

layout(location = 0) out vec4 rgb;

void main(void) {   
    rgb = uni.color0;
}
]]>
    </fragment>
    <pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="true" depthWrite="false" twoSided="true"/>
</shader>
