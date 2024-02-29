<shader>
    <properties>
        <property name="borderWidth" type="vec4"/>
        <property name="borderRadius" type="vec4"/>
        <property name="backgroundColor" type="vec4"/>
        <property name="topColor" type="vec4"/>
        <property name="rightColor" type="vec4"/>
        <property name="bottomColor" type="vec4"/>
        <property name="leftColor" type="vec4"/>
        <property name="borderWidth" type="vec4"/>
    </properties>
    <fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"
#include "Functions.h"

layout(binding = UNIFORM) uniform Uniforms {
    vec4 borderWidth;
    vec4 borderRadius;

    vec4 backgroundColor;
    vec4 topColor;
    vec4 rightColor;
    vec4 bottomColor;
    vec4 leftColor;
} uni;

layout(location = 0) out vec4 rgb;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

void main(void) {   
    bool upperHalf = _uv0.y > 0.5;
    bool rightHalf = _uv0.x > 0.5;

    float ratio = dFdy(_uv0.y) / dFdx(_uv0.x);
    vec2 uvSDF = _uv0;
    uvSDF.x *= ratio;

    float borderRad;
    float borderWidth = uni.borderWidth.x;
    if(rightHalf) {
        uvSDF.x = ratio - uvSDF.x;
    }

    if(upperHalf) {
        uvSDF.y = 1.0 - uvSDF.y;
        borderRad = rightHalf ? uni.borderRadius.y : uni.borderRadius.x;
    } else {
        borderRad = rightHalf ? uni.borderRadius.z : uni.borderRadius.w;
    }

    vec2 radSDF = uvSDF - borderRad;
    float innerSDF = max(0.0, min(radSDF.x, radSDF.y));
    float outerSDF = -length(min(radSDF, 0.0));
    float sdf = innerSDF + outerSDF + borderRad;

    const float softness = 0.0;
    const float margin = 0.0;
    float surface = smoothstep(margin - softness, margin + softness, sdf);

    // Border
    float width = (borderWidth > 0.0) ? max(fwidth(_uv0.y), borderWidth) : 0.0;
    float border = surface - smoothstep(margin, margin, sdf - width);

    float brd0 = step(1.0, ratio * _uv0.x + _uv0.y);
    float brd1 = step(1.0, ratio * (1.0 - _uv0.x) + _uv0.y);
    float brd2 = step(1.0, ratio * (1.0 - _uv0.x) + (1.0 - _uv0.y));
    float brd3 = step(1.0, ratio * (_uv0.x) + (1.0 - _uv0.y));

    float top = brd0 * brd1;
    float right = (1.0 - brd1) * (1.0 - brd2);
    float left = (1.0 - brd0) * (1.0 - brd3);

    vec4 borderColor = mix(mix(mix(uni.bottomColor, uni.leftColor, left), uni.rightColor, right), uni.topColor, top);

    rgb = mix(vec4(uni.backgroundColor.xyz, uni.backgroundColor.w * surface), borderColor, border);
}
]]>
    </fragment>
    <pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="true" depthWrite="false" twoSided="true"/>
</shader>
