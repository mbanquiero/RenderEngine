
float4x4 m_World;
float4x4 m_View;
float4x4 m_Proj;
float4x4 m_WorldViewProj;
float4x4 m_WorldView;
float4x4 m_TransposeInvWorld;

float3 m_LookFrom;

float3 g_LightPos;				// Pos de la luz
float3 g_LightDir;				// Direccion de la luz
float3 g_LightColor;			// Color de la luz

float g_LightPhi;				// Spotlight: cos (Phi angle/2) (cono exterior)	cos(light[l].Phi/2.0f);
float g_LightTheta;				// Spotlight: cos(Theta angle/2) (cono interior)
float4 m_FontColor;

float screen_dx;
float screen_dy;


texture g_Texture;
sampler TextureSampler = 
sampler_state
{
    Texture = <g_Texture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
	float2 texCoords : TEXCOORD0;
    float3 normal: NORMAL;
	float3 wpos : TEXCOORD1;					// world position

};

VS_OUTPUT RenderSceneVS(float4 position : POSITION, float3 normal: NORMAL, float2 texCoords: TEXCOORD0)
{
    VS_OUTPUT output;
	position.w = 1;
    output.position = mul(position, m_World);
    output.position = mul(output.position, m_View);
    output.position = mul(output.position, m_Proj);
    output.normal = normalize(mul(normal, (float3x3)m_World));
	output.texCoords = texCoords;
	float4 pos_real = mul(position, m_World);
	output.wpos = pos_real.xyz;
    return output;
}


float4 RenderScenePS(VS_OUTPUT Input) : COLOR0
{
    float4 base_color = tex2D(TextureSampler,Input.texCoords);
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
	return float4(color,1);
}


technique RenderScene
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderSceneVS();
        PixelShader  = compile ps_3_0 RenderScenePS(); 
    }
}


// ------------------------------------------------------------------------------------------------
float4x4 bonesMatWorldArray[26];

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
    float4 position : POSITION;
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
		skinPosition += mul(Input.position, bonesMatWorldArray[(int)Input.BlendIndices[i]]) * Input.BlendWeights[i];
		skinNormal += mul(Input.normal, (float3x3)bonesMatWorldArray[(int)Input.BlendIndices[i]]) * Input.BlendWeights[i];
		skinTangent += mul(Input.tangent, (float3x3)bonesMatWorldArray[(int)Input.BlendIndices[i]]) * Input.BlendWeights[i];
		skinBinormal += mul(Input.binormal, (float3x3)bonesMatWorldArray[(int)Input.BlendIndices[i]]) * Input.BlendWeights[i];
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


float4 SkeletalPShader(VS_SKIN_OUTPUT Input) : COLOR0
{
    float4 base_color = tex2D(TextureSampler,Input.texCoords);
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

technique SkeletalRender
{
    pass P0
    {          
        VertexShader = compile vs_3_0 SkeletalVShader();
        PixelShader  = compile ps_3_0 SkeletalPShader(); 
    }
}



// -----------------------------------------------
// Vertex Shader para Quad 2d
struct VS_QUAD_OUTPUT
{
    float4 position : POSITION;
	float2 texCoords : TEXCOORD0;
};

VS_QUAD_OUTPUT SpriteVS(float4 position : POSITION, float2 texCoords: TEXCOORD0)
{
    VS_QUAD_OUTPUT output;
	output.position.zw = 1;
	output.position.xy = position.xy;
	output.texCoords.xy = texCoords;
    return output;
}


float4 SpritePS(VS_QUAD_OUTPUT Input) : COLOR0
{
    float4 base_color = tex2D(TextureSampler,Input.texCoords);
	base_color.a *= 0.5;
	return base_color;
}


float4 FontPS(VS_QUAD_OUTPUT Input) : COLOR0
{
    float4 base_color = tex2D(TextureSampler,Input.texCoords);
	if(base_color.r==0 || base_color.a<0.5)
		base_color.a = 0;			// Transparente
	else
	{
		base_color.a = 1;			// font color
		base_color.rgb = m_FontColor;
	}
	return base_color;
}




technique RenderSprite
{
    pass P0
    {          
        VertexShader = compile vs_3_0 SpriteVS();
        PixelShader  = compile ps_3_0 SpritePS(); 
    }
}



technique RenderText
{
    pass P0
    {          
        VertexShader = compile vs_3_0 SpriteVS();
        PixelShader  = compile ps_3_0 FontPS(); 
    }
}
