//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

cbuffer MatrixBuffer : register(b0)
{
    matrix m_World;
    matrix m_View;
    matrix m_Proj;
    matrix m_WorldViewProj;
    matrix m_WorldView;
    matrix m_ViewProj;
	float3 m_LookFrom;
	float time;
	matrix m_TransposeInvWorld;
	float screenWidth;
	float screenHeight;
	float2 pad0;
};

cbuffer LightBuffer : register(b1)
{
	float3 g_LightPos;				// Pos de la luz
	float g_LightPhi;				// Spotlight: cos (Phi angle/2) (cono exterior)	cos(light[l].Phi/2.0f);
	float3 g_LightDir;				// Direccion de la luz
	float g_LightTheta;				// Spotlight: cos(Theta angle/2) (cono interior)
	float3 g_LightColor;			// Color de la luz
	float pad1;
};

cbuffer BoneMatrixBuffer : register(b2)
{
	matrix bonesMatWorldArray[26];
};


cbuffer MaterialBuffer : register(b3)
{
	float k_la;							// luz ambiente global
	float k_ld;							// luz difusa
	float k_ls;							// luz specular
	float pad2;
	float4 fontColor;
};


//--------------------------------------------------------------------------------------
// Structs 
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
    float3 normal: NORMAL;
	float3 wpos : TEXCOORD1;					// world position
	float3 vpos : TEXCOORD2;					// view position
};

// Vertex Shader Mesh Skeletal mesh
struct VS_SKIN_INPUT
{
    float4 position : POSITION;
    float3 normal: NORMAL;
	float2 texCoords : TEXCOORD0;
    float3 tangent: TANGENT;
    float3 binormal: BINORMAL;
	float4 BlendWeights : BLENDWEIGHT;
    float4 BlendIndices : BLENDINDICES;
};

struct VS_SKIN_OUTPUT
{
    float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
    float3 normal: NORMAL;
    float3 tangent: TANGENT;
    float3 binormal: BINORMAL;
	float4 BlendWeights : BLENDWEIGHT;
    float4 BlendIndices : BLENDINDICES;
	float3 wpos : TEXCOORD1;					// world position

};


// Vertex Shader para Quad 2d
struct VS_QUAD_OUTPUT
{
    float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};
