<shader version="11">
    <properties>
        <property binding="0" type="texture2d" name="highMap" target="true"/>
        <property binding="1" type="texture2d" name="lowMap" target="true"/>
        <property binding="2" type="texture2d" name="depthMap" target="true"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#define NO_INSTANCE

#include "ShaderLayout.h"
#include "Functions.h"

layout(binding = UNIFORM) uniform sampler2D highMap;
layout(binding = UNIFORM + 1) uniform sampler2D lowMap;
layout(binding = UNIFORM + 2) uniform sampler2D depthMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 0) out vec3 color;

const vec2 circleOffsets[] = {
    vec2( 0.7071f, -0.7071f ),
    vec2(-0.7071f,  0.7071f ),
    vec2(-1.0000f,  0.0000f ),
    vec2( 1.0000f,  0.0000f ),
    vec2( 0.0000f,  0.0000f ),
    vec2(-0.7071f, -0.7071f ),
    vec2( 0.7071f,  0.7071f ),
    vec2( 0.0000f, -1.0000f ),
    vec2( 0.0000f,  1.0000f )
};

void main(void) {
    vec3 highColor = texture(highMap, _uv0).xyz;
    vec3 lowColor = vec3(0.0f);

    for(int i = 0; i < 9; i++) {
        lowColor += texture(lowMap, _uv0 + g.cameraScreen.zw * circleOffsets[i] ).xyz * 0.1111f;
    }

    float depth = getLinearDepth(texture(depthMap, _uv0).x, g.cameraPosition.w, g.cameraTarget.w);
    float strength = clamp(abs(clamp((depth - 15.0f) * 0.1f, -1.0f, 1.0f)), 0.0f, 1.0f);

    color = mix(highColor, lowColor, strength);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="PostProcess" twoSided="true"/>
</shader>
