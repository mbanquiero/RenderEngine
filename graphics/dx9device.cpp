#include "stdafx.h"
#include "/dev/graphics/dx9device.h"
#include "/dev/graphics/RenderEngine.h"
#include "/dev/graphics/xstring.h"

bool IsSkeletalMesh(char *fname);

CDX9Device::CDX9Device()
{
	g_pD3D = NULL;
	g_pd3dDevice = NULL;
	g_pEffect = NULL;			
	g_pEffectStandard = NULL;	

}

CDX9Device::~CDX9Device()
{
	if(g_pd3dDevice)
		CleanD3D();
}



// Inicializa el DirectX
void  CDX9Device::InitD3D(HWND hWnd)
{
	HRESULT hr;

	// Create the D3D object.
	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
	{
		return;
	}


	// Set up the structure used to create the D3DDevice
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	if(FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice ) ) )
	{
		return;
	}

	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
	g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS , TRUE);
	g_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE);
	g_pd3dDevice->SetRenderState( D3DRS_SHADEMODE,D3DSHADE_GOURAUD);
	g_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, FALSE );

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE);

	// D3DTADDRESS_WRAP
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);		
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR);

	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	// habilito las tranparencias
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_BLENDDIFFUSEALPHA);
	// Color Final = (Source * A) + (Dest * (1-A))
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);		// Source * A
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);	// Dest * (1-A)
	g_pd3dDevice->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);			// Suma ambos terminos
	g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x0000000F);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE); 
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);


	// Inicializo el Viewport
	D3DVIEWPORT9 viewport;
	ZeroMemory(&viewport, sizeof(D3DVIEWPORT9));
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;
	viewport.X = 0;
	viewport.Y = 0;
	screenWidth = viewport.Width = d3dpp.BackBufferWidth;
	screenHeight = viewport.Height = d3dpp.BackBufferHeight;
	g_pd3dDevice->SetViewport(&viewport);

	// transformation pipeline
	fov = (float)D3DX_PI / 4.0f;
	aspect_ratio = (float)screenWidth / (float)screenHeight;
	near_plane = 50;
	far_plane = 50000;

	transponer_matrices = false;
}

void CDX9Device::CleanD3D()
{
	if(g_pD3D==NULL)
		return;

	// Effectos
	SAFE_RELEASE(g_pEffectStandard);

	// Device
	SAFE_RELEASE(g_pd3dDevice);
	SAFE_RELEASE(g_pD3D);
}



void CDX9Device::InitPipeline()
{
	// load and compile shaders
	if(SUCCEEDED(LoadFx(&g_pEffectStandard,"/dev/graphics/shaders9.fx")))
	{
		g_pEffect = g_pEffectStandard;
		g_pEffect->SetTechnique("RenderScene");
	}

	
	// Creo el vertex declaration
	D3DVERTEXELEMENT9 VERTEX_DECL[] =
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
	g_pd3dDevice->CreateVertexDeclaration(VERTEX_DECL, &m_pVertexDeclaration);


	// Creo el vertex declaration
	D3DVERTEXELEMENT9 SKELETAL_VERTEX_DECL[] =
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0}, 
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0}, 
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0},
		{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TANGENT, 0},
		{ 0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BINORMAL, 0},
		{ 0, 56, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDWEIGHT, 0},
		{ 0, 72, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_BLENDINDICES, 0},
		D3DDECL_END()
	};
	g_pd3dDevice->CreateVertexDeclaration(SKELETAL_VERTEX_DECL, &m_pSkeletalMeshVertexDeclaration);

	//g_pd3dDevice->SetFVF( D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1);
}


void CDX9Device::InitTransformPipeline()
{
	// Matrices de Transformacion
	D3DXMatrixPerspectiveFovLH(&m_Proj, fov, aspect_ratio, near_plane, far_plane);
	D3DXMatrixIdentity(&m_World);
}



void CDX9Device::BeginRenderFrame(D3DXMATRIX matView)
{
	// Limpio la pantalla y el depth buffer
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(240,240,240), 1, 0 );

	// Actualizo la matriz de view
	m_View = matView;

	// Y actualizo las variables en el shader de la parte de lighting, que no cambian de frame a frame
	SetShaderLighting();

	g_pd3dDevice->BeginScene();

}

void CDX9Device::EndRenderFrame()
{
	// switch the back buffer and the front buffer
	g_pd3dDevice->EndScene();
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}



void CDX9Device::SetShaderTransform(D3DXVECTOR3 lookFrom)
{
	// matrices de transformacion
	g_pEffect->SetMatrix( "m_World", &m_World);
	g_pEffect->SetMatrix( "m_View", &m_View);
	g_pEffect->SetMatrix( "m_Proj", &m_Proj);

	// punto de vista: 
	g_pEffect->SetValue( "m_LookFrom", &lookFrom, sizeof(D3DXVECTOR3));

	// precalculadas para ganar tiempo: 
	D3DXMATRIXA16 mWorldViewProjection = m_World * m_View * m_Proj;
	g_pEffect->SetMatrix( "m_WorldViewProj", &mWorldViewProjection);
	D3DXMATRIXA16 mWorldView = m_World * m_View;
	g_pEffect->SetMatrix( "m_WorldView", &mWorldView);

	// Transpuesta inversa del world, (para transformar direcciones)		
	D3DXMATRIXA16 m_TransposeInvWorld;
	FLOAT det;
	D3DXMatrixTranspose(&m_TransposeInvWorld,D3DXMatrixInverse(&m_TransposeInvWorld,&det,&m_World));
	g_pEffect->SetMatrix( "m_TransposeInvWorld", &m_TransposeInvWorld);

	// Resolucion de pantalla
	g_pEffect->SetFloat("screen_dx", d3dpp.BackBufferWidth);      
	g_pEffect->SetFloat("screen_dy", d3dpp.BackBufferHeight);      

}

void CDX9Device::SetShaderLighting()
{
	float phi = 2;
	float theta = 0.95;

	D3DXVECTOR3 vLightPos = D3DXVECTOR3(0,2500,0);
	D3DXVECTOR3 vLightDir = D3DXVECTOR3(0,-1,0);
	D3DXVECTOR3 vLightColor = D3DXVECTOR3(1,1,1);

	g_pEffect->SetValue( "g_LightDir", vLightDir, sizeof(D3DXVECTOR3));
	g_pEffect->SetValue( "g_LightPos", vLightPos, sizeof(D3DXVECTOR3));
	g_pEffect->SetValue( "g_LightColor", vLightColor, sizeof(D3DXVECTOR3));
	g_pEffect->SetFloat( "g_LightPhi", cos(phi/2.0));
	g_pEffect->SetFloat( "g_LightTheta", cos(theta/2.0));

//	g_pEffect->SetFloat( "k_la", shader_la);	// luz ambiente
//	g_pEffect->SetFloat( "k_ld", shader_ld);	// luz difusa
//	g_pEffect->SetFloat( "k_ls", shader_ls);	// luz specular
//	g_pEffect->SetFloat( "k_brightness", g_brillo);	
//	g_pEffect->SetFloat( "k_contrast", g_contraste);

}




CBaseTexture *CDX9Device::CreateTexture(char *fname)
{
	CDX9Texture *p_texture = NULL;
	LPDIRECT3DTEXTURE9      g_pTexture;
	if(SUCCEEDED( D3DXCreateTextureFromFile( g_pd3dDevice, fname, &g_pTexture)))
	{
		p_texture = new CDX9Texture();
		strcpy(p_texture->name , fname);
		p_texture->g_pTexture = g_pTexture;
	}
	return (CBaseTexture *)p_texture;
}



CBaseMesh *CDX9Device::LoadMesh(CRenderEngine *p_engine, char *fname)
{
	CBaseMesh *p_mesh = NULL;
	char ext[4];
	que_extension(fname,ext);
	bool ok;
	if(ext[0]=='y' || ext[0]=='Y')
	{

		// Cargo un archivo fomrato .Y 
		p_mesh = (CBaseMesh *)new CDX9Mesh;
		ok = ((CDX9Mesh *)p_mesh)->LoadFromFile(p_engine,this,fname);
	}
	else
	{
		// Cargo un archivo formato xml
		if(IsSkeletalMesh(fname))
		{
			// Skeletal mesh
			p_mesh = new CDX9SkeletalMesh;
			ok = ((CDX9SkeletalMesh*)p_mesh)->LoadFromXMLFile(p_engine,this,fname);
		}
		else
		{
			// Mesh comun 
			p_mesh = new CDX9Mesh;
			ok = ((CDX9Mesh *)p_mesh)->LoadFromXMLFile(p_engine,this,fname);
		}
	}

	if(!ok)
		SAFE_DELETE(p_mesh);			// y p_mesh queda en NULL
	return p_mesh;

}

CBaseMesh *CDX9Device::LoadMeshFromXmlFile(CRenderEngine *p_engine, char *fname,char *mesh_name,int mat_id)
{
	CBaseMesh *p_mesh = NULL;
	p_mesh = new CDX9Mesh;
	bool ok = ((CDX9Mesh *)p_mesh)->LoadFromXMLFile(p_engine,this,fname,mesh_name,mat_id);

	if(!ok)
		SAFE_DELETE(p_mesh);			// y p_mesh queda en NULL

	return p_mesh;
}

HRESULT CDX9Device::LoadFx(ID3DXEffect** ppEffect,char *fx_file)
{
	ID3DXBuffer *pBuffer = NULL;
	HRESULT hr = D3DXCreateEffectFromFile( g_pd3dDevice, fx_file,
		NULL, NULL, D3DXFX_NOT_CLONEABLE, NULL, ppEffect, &pBuffer);
	if( FAILED(hr) )
	{
		char *saux = (char*)pBuffer->GetBufferPointer();
		AfxMessageBox(saux);
	}
	return hr;
}
