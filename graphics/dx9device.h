#pragma once

#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"
#include "/dev/graphics/Camera.h"
#include "/dev/graphics/mesh.h"
#include "/dev/graphics/SkeletalMesh.h"
#include "/dev/graphics/object.h"
#include "/dev/graphics/device.h"
#include "/dev/graphics/texture.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

class CDX9Device : public CDevice
{
public:
	LPDIRECT3D9             g_pD3D;			// Used to create the D3DDevice
	LPDIRECT3DDEVICE9       g_pd3dDevice;			// Our rendering device
	D3DPRESENT_PARAMETERS	d3dpp;

	ID3DXEffect* g_pEffect;				// current Effect
	ID3DXEffect* g_pEffectStandard;	

	LPDIRECT3DVERTEXDECLARATION9 m_pVertexDeclaration;			// Pos + Normal + TexCoords
	LPDIRECT3DVERTEXDECLARATION9 m_pSkeletalMeshVertexDeclaration;



	CDX9Device();
	~CDX9Device();
	virtual void InitD3D(HWND hWnd);
	virtual void CleanD3D();

	virtual void InitPipeline();
	virtual void InitTransformPipeline();

	virtual void BeginRenderFrame(D3DXMATRIX matView);
	virtual void EndRenderFrame();

	virtual void SetShaderTransform(D3DXVECTOR3 lookFrom);		// Transform and...
	virtual void SetShaderLighting();		// Lighting


	virtual CBaseTexture *CreateTexture(char *fname);
	virtual CBaseMesh *LoadMesh(CRenderEngine *p_engine, char *fname);
	virtual CBaseMesh *LoadMeshFromXmlFile(CRenderEngine *p_engine, char *fname,char *mesh_name,int mat_id);

	HRESULT LoadFx(ID3DXEffect** ppEffect,char *fx_file);

};


