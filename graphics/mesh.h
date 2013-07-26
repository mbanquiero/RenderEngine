#pragma once
#include <d3d11.h>
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\dxgi.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dcommon.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx11.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10Math.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"

class CRenderEngine;
class CDevice;
class CDX11Device;
class CDX9Device;

typedef struct MESH_LAYER {
	D3DCOLORVALUE Diffuse;
	D3DCOLORVALUE Ambient;
	float ke;					// coef. luz especular
	float kt;					// transparencia
	float kr;					// reflexion	
	int nro_textura;			// Nro de textura en el pool
	int start_index;
	int cant_indices;
	char texture_name[MAX_PATH];		// diffuse map
} MESH_LAYER;

#define MAX_MESH_LAYERS			32


struct MESH_VERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
};

class CBaseMesh
{
public:
	D3DXVECTOR3 m_pos;
	D3DXVECTOR3 m_size;
	DWORD cant_faces;
	DWORD cant_vertices;
	int cant_indices;
	int bpv;
	DWORD start_vertex;
	DWORD start_index;
	char fname[MAX_PATH];
	char mesh_id[255];
	MESH_LAYER layers[MAX_MESH_LAYERS];
	int cant_layers;
	// Internal Data
	MESH_VERTEX *pVertices;
	DWORD *pIndices;
	DWORD *pAttributes;

	CDevice *device;
	CRenderEngine *engine;

	
	CBaseMesh();
	virtual ~CBaseMesh() = 0;
	virtual void ReleaseInternalData();		// Libera los datos internos, una vez que los Buffers del device estan creados no tiene sentido mantenar esos datos del mesh

	virtual D3DXVECTOR3 pos_vertice(int i) { return pVertices!=NULL ? pVertices[i].position : D3DXVECTOR3(0,0,0);};			// Abstraccion de las posiciones
	virtual bool hay_internal_data() { return pVertices!=NULL ? true : false;};

	virtual void Draw() = 0;
	virtual void SetVertexDeclaration() = 0;
	virtual void SetShaders() = 0;
	virtual bool ComputeBoundingBox();

	virtual bool LoadDataFromFile(char *filename);		// Carga los datos del mesh, esto es independiente del device
	virtual bool LoadFromFile(CRenderEngine *p_engine,CDevice *p_device,char *filename,bool keepData=false);

	virtual bool LoadFromXMLFile(CRenderEngine *p_engine,CDevice *p_device,char *filename,
		char *mesh_name=NULL,int mesh_mat_id=-1);
	
	virtual bool CreateMeshFromData(CRenderEngine *p_engine,CDevice *p_device) = 0;		// Crea el mesh pp dicho desde los datos internos


};



class CDX11Mesh : public CBaseMesh
{
public:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;

	CDX11Mesh();
	virtual ~CDX11Mesh();
	virtual bool CreateMeshFromData(CRenderEngine *p_engine,CDevice *p_device);					// Crea el mesh pp dicho desde los datos internos

	virtual void SetVertexDeclaration();
	virtual void SetShaders();

	virtual void Release();
	virtual void Draw();
	virtual void DrawSubset(int i);


};


class CDX9Mesh : public CBaseMesh
{
public:
	LPDIRECT3DVERTEXBUFFER9 m_vertexBuffer;	
	LPDIRECT3DINDEXBUFFER9 m_indexBuffer;	

	CDX9Mesh();
	virtual ~CDX9Mesh();

	virtual bool CreateMeshFromData(CRenderEngine *p_engine,CDevice *p_device);					// Crea el mesh pp dicho desde los datos internos

	virtual void Release();
	virtual void SetVertexDeclaration();
	virtual void SetShaders();

	virtual void Draw();
	virtual void DrawSubset(int i);

};



