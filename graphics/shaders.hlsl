#include "AdaptiveTessellation.hlsl"

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
// Textures
//--------------------------------------------------------------------------------------
Texture2D g_baseTexture : register( t0 );    // Base color texture
Texture2D g_nmhTexture :  register( t1 );    // Normal map and height map texture pair
StructuredBuffer<float4> buffer : register( t2 );


//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
SamplerState g_samLinear : register( s0 );


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
    float4 base_color = g_baseTexture.Sample(g_samLinear, Input.texCoords);
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

// -----------------------------------------------
// Vertex Shader para Quad 2d
struct VS_QUAD_OUTPUT
{
    float4 position : SV_POSITION;
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


float4 SpritePS(VS_QUAD_OUTPUT Input) : SV_TARGET
{
    //float4 base_color = g_baseTexture.Sample(g_samLinear, Input.texCoords);
	//base_color.a *= 0.5;
	//return base_color;
	float x = Input.texCoords.x * screenWidth;
	float y = Input.texCoords.y * screenHeight;
    return buffer[ (x - 0.5) + (y - 0.5) * screenWidth ];	
}


float4 FontPS(VS_QUAD_OUTPUT Input) : SV_TARGET
{
    float4 base_color = g_baseTexture.Sample(g_samLinear, Input.texCoords);
	if(base_color.r==0 || base_color.a<0.5)
		base_color.a = 0;			// Transparente
	else
	{
		base_color.a = 1;			// font color
		base_color.rgb = fontColor;
	}
	return base_color;
}

// Hardware Tesselation
struct HullInputType
{
    float3 position : WORLDPOS;
    float3 normal : NORMAL;
    float2 texCoords : TEXCOORD0;
    float2 screenPos: TEXCOORD1;
};

// Vertex Shader
HullInputType HT_VShader(float4 position : POSITION, float3 normal: NORMAL, float2 texCoords: TEXCOORD0)
{
    HullInputType output;

    // Compute position in world space
    float4 vPositionWS = mul( position.xyz, m_World);
    float3 vNormalWS   = mul( normal,   (float3x3) m_World);
    vNormalWS   = normalize( vNormalWS );
    // Get the screen space position of each control point
	output.screenPos  = GetScreenSpacePosition( vPositionWS.xyz, m_ViewProj, screenWidth, screenHeight);
	// propago todo
    output.position = vPositionWS.xyz;
    output.normal = vNormalWS;
    output.texCoords = texCoords;
    return output;
}



// Patch Constant Function
struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};
ConstantOutputType MyPatchConstantFunction(InputPatch<HullInputType, 3> p, uint patchId : SV_PrimitiveID)
{    
    ConstantOutputType output;
	float g_fTessellationFactor = 0.1;
    // Calculate edge tessellation factors based on desired screen space tessellation value
    output.edges[0] = g_fTessellationFactor * distance(p[1].screenPos, p[2].screenPos);
    output.edges[1] = g_fTessellationFactor * distance(p[0].screenPos, p[2].screenPos);
    output.edges[2] = g_fTessellationFactor * distance(p[0].screenPos, p[1].screenPos);
	output.inside = 0.33 * ( output.edges[0] + output.edges[1] + output.edges[2] );


	/*
    bool bViewFrustumCull = ViewFrustumCull( p[0].position, p[1].position, p[2].position, g_vFrustumPlaneEquation,
                                             g_vDetailTessellationHeightScale.x );
    if (bViewFrustumCull)
    {
        // Set all tessellation factors to 0 if frustum cull test succeeds
        output.Edges[0] = 0.0;
        output.Edges[1] = 0.0;
        output.Edges[2] = 0.0;
        output.Inside   = 0.0;
    }
	*/

    return output;
}

// Hull Shader
struct HullOutputType
{
    float3 position : WORLDPOS;
    float3 normal : NORMAL;
    float2 texCoords : TEXCOORD0;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("MyPatchConstantFunction")]
[maxtessfactor(64.0)]
HullOutputType sHullShader(InputPatch<HullInputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    HullOutputType output;
    output.position = patch[pointId].position;
    output.normal = patch[pointId].normal;
    output.texCoords = patch[pointId].texCoords;
    return output;
}


// Domain Shader
[domain("tri")]
VS_OUTPUT sDomainShader(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<HullOutputType, 3> patch)
{
    float3 vertexPosition;
	float3 vertexNormal;
	float2 vertexTexCoords;
    VS_OUTPUT output;
    // Determine the position of the new vertex.
    vertexPosition = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;
    vertexNormal = uvwCoord.x * patch[0].normal + uvwCoord.y * patch[1].normal+ uvwCoord.z * patch[2].normal;
	vertexNormal = normalize(vertexNormal);
    vertexTexCoords = uvwCoord.x * patch[0].texCoords + uvwCoord.y * patch[1].texCoords + uvwCoord.z * patch[2].texCoords;

    // Calculate MIP level to fetch from
    float fHeightMapMIPLevel = clamp( ( distance( vertexPosition, m_LookFrom ) - 100.0f ) / 100.0f, 0.0f, 3.0f );

	// Fetch HEIGHT from normal map
    float4 vHeight = g_nmhTexture.SampleLevel( g_samLinear, vertexTexCoords, fHeightMapMIPLevel );

	// dezplazo en direccion a la normal
	vertexPosition += vertexNormal*(vHeight.w*1000);

	//float k = time * 3;
	//float s = 0.01;
	//vertexPosition += vertexNormal*(sin(s*vertexPosition.x + k) + cos(s*vertexPosition.z + k) )*100;

    // Calculate the position of the new vertex against the world, view, and projection matrices.
	matrix g_mViewProjection = mul(m_View, m_Proj);
    output.position = mul(float4(vertexPosition,1), g_mViewProjection);
    output.normal = normalize(mul(vertexNormal, (float3x3)m_World));
	output.texCoords = vertexTexCoords;
	float4 pos_real = mul(float4(vertexPosition,1), m_World);
	output.wpos = pos_real.xyz;
    return output;
}



[maxvertexcount(12)]
void GS_Simple(triangle VS_OUTPUT input[3], inout TriangleStream<VS_OUTPUT> OutputStream)
{
	for(int t = 0;t<4;++t) 
	{	
		for(int i = 0; i < 3; i++)
		{
			input[i].wpos.z += 500;
			input[i].wpos.x += 500;
			input[i].wpos.y += 100;
			input[i].position = mul(float4(input[i].wpos,1), m_WorldViewProj);
    		OutputStream.Append(input[i]);
		}
		OutputStream.RestartStrip();
	}

}

