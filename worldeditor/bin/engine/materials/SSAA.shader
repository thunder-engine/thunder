<Shader>
    <Properties>
        <Property name="rgbMap" type="texture2D" binding="4" target="true"/>
    </Properties>
    <Fragment>
<![CDATA[
#version 450 core

#include "ShaderLayout.h"

layout(binding = UNIFORM) uniform sampler2D rgbMap;

layout(location = 1) in vec2 _uv0;

layout(location = 0) out vec4 rgb;

void main (void) {
    float w = 1.75;
				
    vec2 up = vec2(0.0, g.cameraScreen.y) * w;
    vec2 right = vec2(g.cameraScreen.x, 0.0) * w;
        
    float t = luminanceApprox( texture( rgbMap, _uv0 - up ).xyz );
    float l = luminanceApprox( texture( rgbMap, _uv0 - right ).xyz);
    float r = luminanceApprox( texture( rgbMap, _uv0 + right ).xyz);
    float b = luminanceApprox( texture( rgbMap, _uv0 + up ).xyz );
 
    vec2 n = vec2( -( t - b ), r - l );
    float v = length( n );

    if( v < (1.0 / 16.0) ) {
        rgb = texture( rgbMap, _uv0 );
    } else {
        n *= g.cameraScreen.xy / v;
	 
        vec4 o  = texture( rgbMap, _uv0);
        vec4 t0 = texture( rgbMap, _uv0 + n * 0.5) * 0.9;
        vec4 t1 = texture( rgbMap, _uv0 - n * 0.5) * 0.9;
        vec4 t2 = texture( rgbMap, _uv0 + n) * 0.75;
        vec4 t3 = texture( rgbMap, _uv0 - n) * 0.75;
	 
	rgb = (o + t0 + t1 + t2 + t3) / 4.3;
    }
}
]]>
    </Fragment>
    <Pass type="PostProcess" blendMode="Opaque" lightModel="Unlit" depthTest="false" depthWrite="false" twoSided="true"/>
</Shader>
