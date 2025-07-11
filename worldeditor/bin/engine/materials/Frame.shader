<shader version="11">
    <properties>
        <property type="vec4" name="borderWidth"/>
        <property type="vec4" name="borderRadius"/>
        <property type="vec4" name="backgroundColor"/>
        <property type="vec4" name="topColor"/>
        <property type="vec4" name="rightColor"/>
        <property type="vec4" name="bottomColor"/>
        <property type="vec4" name="leftColor"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"
#include "Functions.h"

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 6) in vec3 _view;
layout(location = 7) flat in vec4 _objectId;
layout(location = 8) flat in int _instanceOffset;
layout(location = 9) in mat4 _modelView;

layout(location = 0) out vec4 rgb;

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

    float brd0 = step(1.0, ratio * _uv0.x + _uv0.y);
    float brd1 = step(1.0, ratio * (1.0 - _uv0.x) + _uv0.y);
    float brd2 = step(1.0, ratio * (1.0 - _uv0.x) + (1.0 - _uv0.y));
    float brd3 = step(1.0, ratio * (_uv0.x) + (1.0 - _uv0.y));

    float top = brd0 * brd1;
    float right = (1.0 - brd1) * (1.0 - brd2);
    float left = (1.0 - brd0) * (1.0 - brd3);

    vec4 borderColor = mix(mix(mix(bottomColor, leftColor, left), rightColor, right), topColor, top);

    rgb = mix(vec4(backgroundColor.xyz, backgroundColor.w * surface), borderColor, border);
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="false"/>
    </pass>
</shader>
