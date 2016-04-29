#include "shaders_inc.hlsl"

// Textures
Texture2D g_baseTexture : register( t0 );    // Base color texture
Texture2D g_nmhTexture :  register( t1 );    // Normal map and height map texture pair

// Samplers
SamplerState g_samLinear : register( s0 );

// Pixel shader para skining (animacion esqueletica)
float4 skeletalPS(VS_SKIN_OUTPUT Input) : SV_TARGET
{
    float4 base_color = g_baseTexture.Sample(g_samLinear, Input.texCoords);
	float k_ld = 0.75;
	float k_ls = 0.6;
	float k_la = 0.3;
	// spot light, emite luz en una direccion especifica
	// Pos = world position
	float3 LightDiffuse = 0;
	float3 LightSpecular = 0;
	float3 N = normalize(Input.normal);
	float3 Tg = normalize(Input.tangent);
	float3 Bn = normalize(Input.binormal);
	float3 vLight = normalize( Input.wpos - g_LightPos);
	float cono = dot( vLight, g_LightDir);
	float K = 0;
	
	// Verifico si el punto cae sobre el cono de luz
	if( cono > g_LightPhi)
	{
		// es iluminado por la luz
		K = 1;
		if( cono <= g_LightTheta)
			// Fall off
			K = (cono-g_LightPhi) / (g_LightTheta-g_LightPhi);
	
		// 1- calcula la luz diffusa
		LightDiffuse = saturate(dot(-vLight,N))*k_ld*K*g_LightColor;
	
		// 2- calcula la luz specular
		float3 D = normalize(Input.wpos-m_LookFrom);
		float ks = saturate(dot(reflect(-vLight,N), D));
		LightSpecular = pow(ks,5)*k_ls*K*g_LightColor;
	}

	// 3- suma luz diffusa + ambient + specular
	float3 color = base_color.xyz*(saturate(k_la+LightDiffuse)) + LightSpecular;
	
	return float4(color,1);
}

