#pragma once
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10Math.h"
#include "/dev/graphics/mesh.h"
#include "/dev/graphics/texture.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
#define SAFE_DELETE(p) { if ( (p) ) { delete (p); (p) = 0; } }

#define MAX_TEXTURAS	256
#define MAX_MESH		256

struct VERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
};


class CRenderEngine
{

public:

	int screenWidth, screenHeight;
	float fov;										// Field of view
	float aspect_ratio;
	float near_plane,far_plane;
	D3DXMATRIX m_World,m_View,m_Proj;
	D3DXMATRIX bonesMatWorldArray[26];					// todo, parche por ahora

	LPDIRECT3D9             g_pD3D;					// Used to create the D3DDevice
	LPDIRECT3DDEVICE9       g_pd3dDevice;			// Our rendering device
	D3DPRESENT_PARAMETERS	d3dpp;
	LPD3DXFONT				g_pFont;
	LPD3DXSPRITE			g_pSprite;		

	ID3DXEffect* g_pEffect;				// current Effect
	ID3DXEffect* g_pEffectStandard;	

	LPDIRECT3DVERTEXDECLARATION9 m_pVertexDeclaration;			// Pos + Normal + TexCoords
	LPDIRECT3DVERTEXDECLARATION9 m_pSkeletalMeshVertexDeclaration;
	LPDIRECT3DVERTEXDECLARATION9 m_pSpriteVertexDeclaration;	// Pos + TexCoords

	// Camera support
	D3DXVECTOR3 lookFrom , lookAt;

	// Pool de texturas
	int cant_texturas;
	CTexture *m_texture[MAX_TEXTURAS];

	// Pool de meshes
	int cant_mesh;
	CMesh *m_mesh[MAX_MESH];

	float total_time;
	float elapsed_time;

	int cant_frames;		// frame count
	float ftime;			// frame time
	float fps;				


	CRenderEngine();
	~CRenderEngine();

	// inicializacion y cleanup
	void Create(HWND hWnd);		
	void InitD3D(HWND hWnd);
	void InitPipeline();
	bool IsReady();
	void Release();			// closes Direct3D and releases memory


	void Update(float p_elpased_time);
	void RenderFrame(void (*lpfnRender)()=NULL);
	int LoadTexture(char *filename);
	void ReleaseTextures();
	int LoadMesh(char *filename);
	void ReleaseMeshes();

	// Soporte de archivos xml
	bool LoadSceneFromXml(char *filename);

	virtual void SetShaderTransform();		// Transform and...
	virtual void SetShaderLighting();		// Lighting


	virtual CTexture *CreateTexture(char *fname);
	

	HRESULT LoadFx(ID3DXEffect** ppEffect,char *fx_file);

	// Render states
	virtual void SetZEnabled(bool enabled=true);
	virtual void SetAlphaBlendEnabled(bool enabled=true);

	// Text
	virtual void TextOut(int x,int y,char *string);



private:
	virtual CMesh *LoadMeshFromFile(char *fname);
	virtual CMesh *LoadMeshFromXmlFile(char *fname,char *mesh_name,int mat_id);


};

bool IsSkeletalMesh(char *fname);
void rotar_xz(D3DXVECTOR3 *V , float an);

