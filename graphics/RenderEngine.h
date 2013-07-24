#pragma once

#pragma pack(1)


#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10Math.h"
#include "/dev/graphics/device.h"
#include "/dev/graphics/Camera.h"
#include "/dev/graphics/mesh.h"
#include "/dev/graphics/texture.h"
#include "/dev/graphics/object.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
#define SAFE_DELETE(p) { if ( (p) ) { delete (p); (p) = 0; } }

#define MAX_TEXTURAS	256
#define MAX_MESH		256
#define MAX_OBJ			4000

#define DEVICE_DX11		0
#define DEVICE_DX9		1

struct VERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
};



class CRenderEngine
{
public:
	CDevice *device;

	// Camera support
	CBaseCamera *cur_camera;
	CRotCamera rot_camera;

	// Pool de texturas
	int cant_texturas;
	CBaseTexture *m_texture[MAX_TEXTURAS];

	// Pool de meshes
	int cant_mesh;
	CBaseMesh *m_mesh[MAX_MESH];

	// Objetos pp dichos
	int cant_obj;
	CGraphicObject *m_obj[MAX_OBJ];

	float total_time;
	float elpased_time;

	CRenderEngine();
	~CRenderEngine();

	void Create(HWND hWnd,int version);		
	bool IsReady();

	void InitD3D(HWND hWnd);     // sets up and initializes Direct3D
	void ClearScene();			// Limpia todos los objetos (pero no libera la memoria del DIRECTX)
	void CleanD3D();			// closes Direct3D and releases memory
	void Update(float p_elpased_time);
	void RenderFrame();
	void InitPipeline();
	void InitGraphics();
	void SetMatrixBuffer();
	void SetLightBuffer();
	int LoadTexture(char *filename);
	void ReleaseTextures();
	int LoadMesh(char *filename);
	void ReleaseMeshes();
	void ReleaseObj();

	// Transform pipeline
	void SetTransformWorld(D3DXMATRIX matWorld);
	void SetTransformView(D3DXMATRIX matView);
	void SetTransformProj(D3DXMATRIX matProj);

	// Picking
	//Vector3 que_punto(CPoint pt);

	// Interface de creacion de objetos
	CGraphicObject *CreateMesh(char *filename,D3DXVECTOR3 pos,D3DXVECTOR3 size=D3DXVECTOR3(0,0,0),D3DXVECTOR3 rot=D3DXVECTOR3(0,0,0));


	// Soporte de archivos xml
	bool LoadSceneFromXml(char *filename);


};

// Helpers
char is_file(char *file_name);
bool esta_vacio(char *s);
void extension(char *file,char *ext);
char *rtrim(char *string);
char *ltrim(char *string);
char *que_extension(char *file,char *ext);
bool IsSkeletalMesh(char *fname);
