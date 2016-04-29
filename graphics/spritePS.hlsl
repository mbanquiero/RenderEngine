#include "shaders_inc.hlsl"

// Textures
Texture2D g_baseTexture : register( t0 );    // Base color texture
Texture2D g_nmhTexture :  register( t1 );    // Normal map and height map texture pair
// Samplers
SamplerState g_samLinear : register( s0 );


StructuredBuffer<float4> buffer : register( t2 );		// Output buffer

// -----------------------------------------------
// Pixel Shader para sprites 
// -----------------------------------------------
float4 spritePS(VS_QUAD_OUTPUT Input) : SV_TARGET
{
	float4 base_color = g_baseTexture.Sample(g_samLinear, Input.texCoords);
	base_color.a *= 0.5;
	return base_color;


/*	float x = Input.texCoords.x * screenWidth;
	float y = Input.texCoords.y * screenHeight;
    return buffer[ (x - 0.5) + (y - 0.5) * screenWidth ];	
	*/
}


