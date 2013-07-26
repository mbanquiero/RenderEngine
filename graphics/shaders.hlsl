
cbuffer MatrixBuffer : register(b0)
{
    matrix m_World;
    matrix m_View;
    matrix m_Proj;
    matrix m_WorldViewProj;
    matrix m_WorldView;
	float3 m_LookFrom;
	float pad0;
	matrix m_TransposeInvWorld;
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
};


Texture2D shaderTexture;
SamplerState SampleType;


struct VS_OUTPUT
{
    float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
    float3 normal: NORMAL;
	float3 wpos : TEXCOORD1;					// world position

};

// Vertex Shader Mesh comun y corriente
VS_OUTPUT VShader(float4 position : POSITION, float3 normal: NORMAL, float2 texCoords: TEXCOORD0)
{
    VS_OUTPUT output;
	position.w = 1;
    output.position = mul(position, m_WorldViewProj);
    output.normal = normalize(mul(normal, (float3x3)m_World));
	output.texCoords = texCoords;
	float4 pos_real = mul(position, m_World);
	output.wpos = pos_real.xyz;
    return output;
}


float4 PShader(VS_OUTPUT Input) : SV_TARGET
{
    float4 base_color = shaderTexture.Sample(SampleType, Input.texCoords);
	float k_ld = 0.75;
	float k_ls = 0.6;
	float k_la = 0.3;
	// spot light, emite luz en una direccion especifica
	// Pos = world position
	float3 LightDiffuse = 0;
	float3 LightSpecular = 0;
	float3 N = normalize(Input.normal);
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
	return float4(color.xyz,1);
}



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

VS_SKIN_OUTPUT SkeletalVShader(VS_SKIN_INPUT Input)
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


float4 SkeletalPShader(VS_SKIN_OUTPUT Input) : SV_TARGET
{
    float4 base_color = shaderTexture.Sample(SampleType, Input.texCoords);
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

