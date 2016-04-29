#include "shaders_inc.hlsl"


VS_SKIN_OUTPUT skeletalVS(VS_SKIN_INPUT Input)
{
    VS_SKIN_OUTPUT output;
	Input.position.w = 1;

	//Skinning 
	float4 skinPosition = (float4)0;
	float3 skinNormal = (float3)0;
	float3 skinTangent = (float3)0;
	float3 skinBinormal = (float3)0;
	for(int i=0;i<4;++i)
	{
		skinPosition += mul(Input.position, bonesMatWorldArray[(uint)Input.BlendIndices[i]]) * Input.BlendWeights[i];
		skinNormal += mul(Input.normal, (float3x3)bonesMatWorldArray[(uint)Input.BlendIndices[i]]) * Input.BlendWeights[i];
		skinTangent += mul(Input.tangent, (float3x3)bonesMatWorldArray[(uint)Input.BlendIndices[i]]) * Input.BlendWeights[i];
		skinBinormal += mul(Input.binormal, (float3x3)bonesMatWorldArray[(uint)Input.BlendIndices[i]]) * Input.BlendWeights[i];
	}

	// Proyecto la posicion 
	output.position = mul(skinPosition, m_WorldViewProj);

	// Propago la normal, tangent y binormal en world space
    output.normal = normalize(mul(skinNormal, (float3x3)m_TransposeInvWorld));
    output.tangent= normalize(mul(skinTangent, (float3x3)m_TransposeInvWorld));
    output.binormal = normalize(mul(skinBinormal, (float3x3)m_TransposeInvWorld));

	// Propago texturas y blend para debug
	output.texCoords = Input.texCoords;
	output.BlendWeights = Input.BlendWeights;
	output.BlendIndices = Input.BlendIndices;

	// propago la pos en world space para computos de lighting 
	float4 pos_real = mul(Input.position, m_World);
	output.wpos = pos_real.xyz;
    return output;
}
