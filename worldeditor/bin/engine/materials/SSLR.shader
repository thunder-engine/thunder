<Shader>
    <Properties>
        <Property name="depthMap" type="texture2D" binding="1" target="true"/>
        <Property name="normalsMap" type="texture2D" binding="2" target="true"/>
        <Property name="paramsMap" type="texture2D" binding="3" target="true"/>
        <Property name="emissiveMap" type="texture2D" binding="4" target="true"/>
    </Properties>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM + 1) uniform sampler2D depthMap;
layout(binding = UNIFORM + 2) uniform sampler2D normalsMap;
layout(binding = UNIFORM + 3) uniform sampler2D paramsMap;
layout(binding = UNIFORM + 4) uniform sampler2D emissiveMap;

layout(location = 0) in vec4 _vertex;
layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 color;

const float epsilon = 0.00001;

const int marchSteps = 100;
const float marchIncrease = 1.0;

const int searchSteps = 32;
const float searchDecrease = 0.5;

vec2 binarySearch(vec3 pos, vec3 dir) {
    for(int i = 0; i < searchSteps; i++) {
        float depth = textureLod(depthMap, pos.xy, 0.0).x;
        float delta = pos.z - depth;
        if(abs(delta) < epsilon) {
            break;
        }
        
        if(delta < 0.0) {
            pos += dir;
        } else {
            pos -= dir;
        }
            
        dir *= searchDecrease;
    }
    return pos.xy;
}

vec3 rayMarch(vec3 pos, vec3 dir, float step) {
    vec3 prevRay = pos;
    
    float l = step;
    float error = 1.0 - l;
    for(int i = 0; i < marchSteps; i++) {
        if(pos.x < 0.0 || pos.x > 1.0 || pos.y < 0.0 || pos.y > 1.0) {
            break;
        }
        float depth = textureLod(depthMap, pos.xy, 0.0).x;
        float delta = pos.z - depth;
        if(delta > 0.0) {
            return vec3(binarySearch((prevRay + pos) * 0.5, dir * l), error);
        }
        
        prevRay = pos;
        pos += dir * l;
        l = l * marchIncrease;
        error *= 1.0 - l;
    }
    return vec3(0.0, 0.0, 0.0);
}

void main(void) {
    float depth = texture(depthMap, _uv0).x;
    color = vec4(0.0);
    if(depth < 1.0) {
        vec4 normals = texture(normalsMap, _uv0);

        vec4 params = texture(paramsMap, _uv0);
        float rough = params.x;
        if(normals.w == 0.0 || rough > 0.8) {
            return;
        }

        vec3 origin = vec3(_uv0, depth);
        vec3 world = getWorld(g.cameraScreenToWorld, origin.xy, origin.z);
		
        vec3 v = normalize(world - g.cameraPosition.xyz);
        vec3 n = normals.xyz * 2.0 - 1.0;
        vec3 refl = reflect(v, n);

        vec4 ray = g.cameraWorldToScreen * vec4(world + refl, 1.0);
        ray /= ray.w;
        ray.xyz = ray.xyz * 0.5 + 0.5;
        
        vec3 dir = normalize(ray.xyz - origin);
        vec3 coord = rayMarch(origin, dir, 1.0 / marchSteps);
        color = vec4(texture(emissiveMap, coord.xy).xyz, coord.z);
    }
}
]]>
    </Fragment>
    <Pass type="PostProcess" blendMode="Opaque" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>
