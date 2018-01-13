/*============================================================================
Original source taken from

                    NVIDIA FXAA 3.11 by TIMOTHY LOTTES

and adapted for ThunderEngine

------------------------------------------------------------------------------                       
COPYRIGHT (C) 2010, 2011 NVIDIA CORPORATION. ALL RIGHTS RESERVED.
------------------------------------------------------------------------------                       
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED 
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS 
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL NVIDIA 
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR 
CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR 
LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, 
OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE 
THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGES.

============================================================================*/

#pragma version

#include ".embedded/Common.vert"

uniform sampler2D   rgbMap;

layout(location = 0) in vec2 _uv;

out vec4 rgb;

void main (void) {
    float lumaNw = luminanceApprox( texture2D(rgbMap, extents.xy) );
    float lumaSw = luminanceApprox( texture2D(rgbMap, extents.xw) );
    float lumaNe = luminanceApprox( texture2D(rgbMap, extents.zy) );
    float lumaSe = luminanceApprox( texture2D(rgbMap, extents.zw) );
    
    vec3 centre = texture2D(rgbMap, pos).rgb;
    float lumaCentre = luminanceApprox(centre);
    
    float lumaMaxNwSw = max( lumaNw, lumaSw );
    lumaNe += 1.0/384.0;
    float lumaMinNwSw = min( lumaNw, lumaSw );
    
    float lumaMaxNeSe = max( lumaNe, lumaSe );
    float lumaMinNeSe = min( lumaNe, lumaSe );
    
    float lumaMax = max( lumaMaxNeSe, lumaMaxNwSw );
    float lumaMin = min( lumaMinNeSe, lumaMinNwSw );
    
    float lumaMaxScaled = lumaMax * _EdgeThreshold;
    
    float lumaMinCentre = min( lumaMin, lumaCentre );
    float lumaMaxScaledClamped = max( _EdgeThresholdMin, lumaMaxScaled );
    float lumaMaxCentre = max( lumaMax, lumaCentre );
    float dirSWMinusNE = lumaSw - lumaNe;
    float lumaMaxCMinusMinC = lumaMaxCentre - lumaMinCentre;
    float dirSEMinusNW = lumaSe - lumaNw;
    
    if(lumaMaxCMinusMinC < lumaMaxScaledClamped) {
        rgb = centre;
    } else {
        vec2 dir(dirSWMinusNE + dirSEMinusNW, dirSWMinusNE - dirSEMinusNW);
       
        dir = normalize(dir);			
        vec3 col1 = texture2D(rgbMap, pos.xy - dir * rcpSize.zw).rgb;
        vec3 col2 = texture2D(rgbMap, pos.xy + dir * rcpSize.zw).rgb;
        
        float dirAbsMinTimesC = min( abs( dir.x) , abs( dir.y ) ) * _EdgeSharpness;
        dir = clamp(dir.xy/dirAbsMinTimesC, -2.0, 2.0);
        
        vec3 col3 = texture2D(rgbMap, pos.xy - dir * rcpSize2.zw).rgb;
        vec3 col4 = texture2D(rgbMap, pos.xy + dir * rcpSize2.zw).rgb;
        
        vec3 rgbyA = col1 + col2;
        vec3 rgbyB = ((col3 + col4) * 0.25) + (rgbyA * 0.25);
        
        if((luminanceApprox(rgbyA) < lumaMin) || (luminanceApprox(rgbyB) > lumaMax)) {
            rgb = rgbyA * 0.5;
        } else {
            rgb = rgbyB;
        }
    }
}                
