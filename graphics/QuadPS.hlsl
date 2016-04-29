#include "shaders_inc.hlsl"


/*
StructuredBuffer<float4> buffer : register( t0 );		// Input buffer = Output del Compute Shader

float4 quadPS(VS_QUAD_OUTPUT Input) : SV_TARGET
{
	float x = Input.texCoords.x * screenWidth;
	float y = Input.texCoords.y * screenHeight;
    return buffer[ (x - 0.5) + (y - 0.5) * screenWidth ];	
}

*/

// Experimento para VOXELIZAR
#define GRID_X			1024.0f
#define GRID_Y			1024.0f
#define GRID_Z			32.0f


StructuredBuffer<uint> grid: register( t0 );

float4 quadPS(VS_QUAD_OUTPUT Input) : SV_TARGET
{
	uint x = Input.texCoords.x * GRID_X;
	uint y = Input.texCoords.y * GRID_Y;
	uint p = y*GRID_Z*GRID_X + x*GRID_Z;
	int cant = 0;
	for(int i = 0 ; i<GRID_Z ; ++i)
	{
		uint w = grid[p+i];
		for(int t=0;t<32;++t)
		{
			cant += w&1;
			w = w >> 1;
		}
	}
	float k = (float)cant / 8.0f;
    return float4(k,k,k,1);	
}
