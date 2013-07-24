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
#include "/dev/graphics/SkeletalMesh.h"
#include "/dev/graphics/object.h"
#include "/dev/graphics/device.h"
#include "/dev/graphics/texture.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

// Constant buffers
struct MatrixBufferType
{
	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX projection;
	D3DXMATRIX worldViewProj;
	D3DXMATRIX worldView;
	D3DXVECTOR3 lookFrom;
	FLOAT padding;
	D3DXMATRIX TransInvWorld;

};

// Guarda!! tiene que ser multiplo de 4 floats si no se pudre todo y no carga el buffer, al final hay que agregarle un padding para completar
struct LightBufferType
{
	D3DXVECTOR3 Pos;
	FLOAT Phi;
	D3DXVECTOR3 Dir;
	FLOAT Theta;
	D3DXVECTOR3 Color;
	FLOAT padding;
};


struct boneBufferType
{
	D3DXMATRIX bonesMatWorldArray[26];
};


class CDX11Device : public CDevice
{
public:
	IDXGISwapChain *swapchain;             
	ID3D11Device *dev;                     
	ID3D11DeviceContext *devcon;
	ID3D11RenderTargetView *m_renderTargetView;
	ID3D11VertexShader *pVS,*pSkeletalMeshVS;    // vertex shaders
	ID3D11PixelShader *pPS,*pSkeletalMeshPS;     // pixel shaders
	ID3D11Buffer *m_matrixBuffer;		// Constant buffer de matrices
	ID3D11Buffer *m_lightBuffer;		// Constant buffer de lighting
	ID3D11Buffer *m_boneBuffer;			// Constant matrices de animacion
	ID3D11InputLayout *pLayout;			// Pos + Normal + TexCoords
	ID3D11InputLayout *pLayoutSkeletalMesh;			// Pos + Normal + TexCoords + binormal + tangent + blend + weights
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;
	ID3D11SamplerState* m_sampleState;



	CDX11Device();
	~CDX11Device();
	virtual void InitD3D(HWND hWnd);
	virtual void CleanD3D();

	virtual void InitPipeline();
	virtual void InitTransformPipeline();

	virtual void BeginRenderFrame(D3DXMATRIX matView);
	virtual void EndRenderFrame();

	virtual void SetShaderTransform(D3DXVECTOR3 lookFrom);
	virtual void SetShaderLighting();

	virtual CBaseTexture *CreateTexture(char *fname);
	virtual CBaseMesh *LoadMesh(CRenderEngine *p_engine, char *fname);
	virtual CBaseMesh *LoadMeshFromXmlFile(CRenderEngine *p_engine, char *fname,char *mesh_name);


};