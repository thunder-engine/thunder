<shader version="11">
    <properties>
        <property type="vec4" name="color0"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform Uniforms {
        vec4 color0;
} uni;

layout(location = 0) out vec4 rgb;

void main(void) {   
    rgb = uni.color0;
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="true"/>
    </pass>
</shader>
