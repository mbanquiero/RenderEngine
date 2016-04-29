#include "shaders_inc.hlsl"

//--------------------------------------------------------------------------------------
// Vertex shader standard
//--------------------------------------------------------------------------------------

VS_OUTPUT standarVS(float4 position : POSITION, float3 normal: NORMAL, float2 texCoords: TEXCOORD0)
{
    VS_OUTPUT output;
	position.w = 1;
    output.position = mul(position, m_WorldViewProj);
    output.normal = normalize(mul(normal, (float3x3)m_World));
	output.texCoords = texCoords;
	float4 pos_real = mul(position, m_World);
	output.wpos = pos_real.xyz;
	float4 pos_view = mul(position, m_WorldView);
	output.vpos = pos_view.xyz;
    return output;
}
