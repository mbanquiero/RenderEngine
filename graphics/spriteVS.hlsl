#include "shaders_inc.hlsl"


// -----------------------------------------------
// Vertex Shader para Quad 2d

VS_QUAD_OUTPUT spriteVS(float4 position : POSITION, float2 texCoords: TEXCOORD0)
{
    VS_QUAD_OUTPUT output;
	output.position.zw = 1;
	output.position.xy = position.xy;
	output.texCoords.xy = texCoords;
    return output;
}
