<Shader>
    <Properties>
        <Property name="color0" type="vec4"/>
        <Property name="borderColor" type="vec4"/>
        <Property name="cornerRadius" type="vec4"/>
        <Property name="borderWidth" type="float"/>
    </Properties>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform Uniforms {
        vec4 color0;
        vec4 borderColor;
        vec4 cornerRadius;
        float borderWidth;
} uni;

layout(location = 0) out vec4 rgb;

layout(location = 1) in vec2 _uv0;

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
    </Fragment>
    <Pass type="Surface" blendMode="Translucent" lightModel="Unlit" depthTest="true" depthWrite="false" twoSided="true"/>
</Shader>
