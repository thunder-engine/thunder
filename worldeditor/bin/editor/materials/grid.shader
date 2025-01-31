<shader version="11">
    <properties>
        <property type="vec4" name="mainColor"/>
        <property type="int" name="scale"/>
        <property type="float" name="width"/>
        <property type="int" name="orientation"/>
        <property type="bool" name="ortho"/>
    </properties>
    <fragment><![CDATA[
#version 450 core

#pragma flags

#include "ShaderLayout.h"

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;

layout(location = 6) in vec3 _view;
layout(location = 7) flat in vec4 _objectId;
layout(location = 8) flat in int _instanceOffset;
layout(location = 9) in mat4 _modelView;

layout(location = 0) out vec4 rgb;

const int subItems = 10;

const vec3 red = vec3(0.62f, 0.23f, 0.3f);
const vec3 green = vec3(0.4f, 0.55f, 0.14f);
const vec3 blue = vec3(0.22f, 0.4f, 0.6f);

const int XZ = 0;
const int XY = 1;
const int ZY = 2;

void main() {
#pragma instance

    float lineWidth = width;

    vec4 world = g.cameraScreenToWorld * _vertex;
    world.xyz / world.w;

    vec2 offset = world.xz;
    vec3 xColor = blue;
    vec3 yColor = red;

    if(orientation == XY) {
        offset = world.xy;
        xColor = green;
    } else if(orientation == ZY) {
        offset = world.zy;
        xColor = green;
        yColor = blue;
    }

    bool cellCondition = (mod(offset.x, scale) <= lineWidth || mod(offset.y, scale) <= lineWidth);
    bool clusterCondition = (mod(offset.x, scale * subItems) <= lineWidth || mod(offset.y, scale * subItems) <= lineWidth);

    if(cellCondition) {
        vec3 pos = _vertex.xyz / _vertex.w;

        float fog = mainColor.w;
        if(!ortho) {
            fog = clamp(mainColor.w * 100.0f * (1.0f - pos.z), 0.0f, 1.0f);
        }

        if(clusterCondition) {
            vec3 color = mainColor.xyz;

            if(offset.x > 0.0f && offset.x < lineWidth) {
                color = xColor;
            }

            if(offset.y > 0.0f && offset.y < lineWidth) {
                color = yColor;
            }

            rgb = vec4(color, fog);
        } else {
            rgb = vec4(mainColor.xyz, fog * 0.4f);
        }
    } else {
        discard;
    }
}
]]></fragment>
    <pass wireFrame="false" lightModel="Unlit" type="Surface" twoSided="true">
        <blend src="SourceAlpha" dst="OneMinusSourceAlpha" op="Add"/>
        <depth comp="Less" write="false" test="true"/>
    </pass>
</shader>
