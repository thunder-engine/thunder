<Shader>
    <Properties>
        <Property name="depthMap" type="texture2D" binding="1" target="true"/>
        <Property name="normalsMap" type="texture2D" binding="2" target="true"/>
        <Property name="paramsMap" type="texture2D" binding="3" target="true"/>
        <Property name="rgbMap" type="texture2D" binding="4" target="true"/>
        <Property name="environmentMap" type="samplerСube" binding="5"/>
    </Properties>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM + 1) uniform sampler2D depthMap;
layout(binding = UNIFORM + 2) uniform sampler2D normalsMap;
layout(binding = UNIFORM + 3) uniform sampler2D paramsMap;
layout(binding = UNIFORM + 4) uniform sampler2D rgbMap;
layout(binding = UNIFORM + 5) uniform samplerCube environmentMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 color;

vec3 boxProjection(vec3 dir) {
    return dir;
}

void main(void) {
    float depth = texture(depthMap, _uv0).x;
    if(depth < 1.0) {
        vec3 origin = vec3(_uv0, depth);
        vec3 world = getWorld(g.cameraScreenToWorld, origin.xy, origin.z);

        vec3 v = normalize(world - g.cameraPosition.xyz);
        vec3 n = texture(normalsMap, _uv0).xyz * 2.0 - 1.0;
        vec3 refl = reflect(v, n);
        
        refl = boxProjection(refl);
        
        vec4 params = texture(paramsMap, _uv0);
        float rough = params.x;

        vec3 ibl = textureLod(environmentMap, refl, rough * 10.0).xyz;

        vec4 sslr = texture(rgbMap, _uv0);
        color = vec4(mix(ibl, sslr.xyz, sslr.w), (1.0 - rough));
        return;
    }
    color = vec4(0.0);
}
]]>
    </Fragment>
    <Pass type="PostProcess" blendMode="Opaque" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>
