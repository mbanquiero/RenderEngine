#pragma once

#pragma pack(1)

#include <d3d11.h>
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\dxgi.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dcommon.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx11.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10Math.h"
#include "/dev/graphics/Camera.h"
#include "/dev/graphics/mesh.h"
#include "/dev/graphics/object.h"
#include "/dev/graphics/texture.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

class CBaseMesh;

class CDevice
{
public:
	// Transforma Pipeline
	int screenWidth, screenHeight;
	float fov;										// Field of view
	float aspect_ratio;
	float near_plane,far_plane;
	D3DXMATRIX m_World,m_View,m_Proj;
	bool transponer_matrices;						// hay que transponer las matrices para el shader
	// todo, parche por ahora
	D3DXMATRIX bonesMatWorldArray[26];

	virtual void InitD3D(HWND hWnd) = 0;
	virtual void CleanD3D()= 0;

	virtual void InitPipeline()= 0;
	virtual void InitTransformPipeline()= 0;

	virtual void BeginRenderFrame(D3DXMATRIX matView)= 0;
	virtual void EndRenderFrame()= 0;

	virtual void SetShaderTransform(D3DXVECTOR3 lookFrom)= 0;
	virtual void SetShaderLighting()= 0;

	virtual CBaseTexture *CreateTexture(char *fname)= 0;
	virtual CBaseMesh *LoadMesh(CRenderEngine *p_engine, char *fname) = 0;
	virtual CBaseMesh *LoadMeshFromXmlFile(CRenderEngine *p_engine, char *fname,char *mesh_name) = 0;
	
};

