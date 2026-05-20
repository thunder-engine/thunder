<?xml version="1.0"?>
<shader version="14">
    <properties>
        <property name="borderWidth" type="vec4" />
        <property name="borderRadius" type="vec4" />
        <property name="backgroundColor" type="vec4" />
        <property name="borderColor" type="vec4" />
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 6) flat in vec4 _objectId;
layout(location = 7) flat in int _instanceOffset;

layout(location = 0) out vec4 rgb;

#include "ShaderLayout.h"
#include "Functions.h"

void main(void) {
#pragma instance

    bool upperHalf = _uv0.y > 0.5;
    bool rightHalf = _uv0.x > 0.5;

    float ratio = abs(dFdy(_uv0.y)) / abs(dFdx(_uv0.x));
    vec2 uvSDF = _uv0;
    uvSDF.x *= ratio;

    float borderRad;
    if(rightHalf) {
        uvSDF.x = ratio - uvSDF.x;
    }

    if(upperHalf) {
        uvSDF.y = 1.0 - uvSDF.y;
        borderRad = rightHalf ? borderRadius.y : borderRadius.x;
    } else {
        borderRad = rightHalf ? borderRadius.z : borderRadius.w;
    }

    vec2 radSDF = uvSDF - borderRad;
    float innerSDF = max(0.0, min(radSDF.x, radSDF.y));
    float outerSDF = -length(min(radSDF, 0.0));
    float sdf = innerSDF + outerSDF + borderRad;

    const float softness = 0.001;
    float surface = smoothstep(softness, softness, sdf);

    // Border
    float border = surface - smoothstep(softness, softness, sdf - borderWidth.x);

    rgb = mix(vec4(backgroundColor.xyz, backgroundColor.w * surface), borderColor, border);
}
]]></fragment>
    <pass type="Surface" twoSided="true" lightModel="Unlit" wireFrame="false">
        <blend op="Add" dst="OneMinusSourceAlpha" src="SourceAlpha" />
    </pass>
</shader>
