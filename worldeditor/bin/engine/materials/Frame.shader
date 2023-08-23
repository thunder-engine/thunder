<shader>
    <properties>
        <property name="color0" type="vec4"/>
        <property name="borderColor" type="vec4"/>
        <property name="cornerRadius" type="vec4"/>
        <property name="borderWidth" type="float"/>
    </properties>
    <fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"
#include "Functions.h"

layout(binding = UNIFORM) uniform Uniforms {
        vec4 color0;
        vec4 borderColor;
        vec4 cornerRadius;
        float borderWidth;
} uni;

layout(location = 0) out vec4 rgb;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;
layout(location = 2) in vec4 _color;
layout(location = 3) in vec3 _n;
layout(location = 4) in vec3 _t;
layout(location = 5) in vec3 _b;

void main(void) {   
    bool upper_half = _uv0.y > 0.5;
    bool right_half = _uv0.x > 0.5;
    float corner_rad;

    float ratio = dFdy(_uv0.y) / dFdx(_uv0.x);
    vec2 uv_sdf = _uv0;
    uv_sdf.x *= ratio;

    if(right_half) {
        uv_sdf.x = ratio - uv_sdf.x;
    }
    if(upper_half) {
        uv_sdf.y = 1.0 - uv_sdf.y;
        corner_rad = right_half ? uni.cornerRadius.z : uni.cornerRadius.w;
    } else {
        corner_rad = right_half ? uni.cornerRadius.y : uni.cornerRadius.x;
    }

    uv_sdf -= corner_rad;
    float inner_sdf = max(0.0, min(uv_sdf.x, uv_sdf.y));
    float outer_sdf = -length(min(uv_sdf, 0.0));
    float sdf = inner_sdf + outer_sdf + corner_rad;

    const float softness = 0.0;
    const float margin = 0.0;
    float surface = smoothstep(margin - softness, margin + softness, sdf);

    // Border
    float width = (uni.borderWidth > 0.0) ? max(fwidth(_uv0.y), uni.borderWidth) : 0.0;
    float border = surface - smoothstep(margin, margin, sdf - width);

    rgb = mix(vec4(uni.color0.xyz, uni.color0.w * surface), uni.borderColor, border);
}
]]>
    </fragment>
    <pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="true" depthWrite="false" twoSided="true"/>
</shader>
