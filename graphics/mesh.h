#pragma once
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\dxgi.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dcommon.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"

class CRenderEngine;

typedef struct MESH_LAYER {
	D3DCOLORVALUE Diffuse;
	D3DCOLORVALUE Ambient;
	float ke;					// coef. luz especular
	float kt;					// transparencia
	float kr;					// reflexion	
	int nro_textura;			// Nro de textura en el pool (Diffusemap)
	int nro_normal_map;			// Nro de textura en el pool (NormalHeightMap)
	int start_index;
	int cant_indices;
	char texture_name[MAX_PATH];					// diffuse map
	char normal_heightmap_name[MAX_PATH];			// NormalHeightMap
} MESH_LAYER;

#define MAX_MESH_LAYERS			32


struct MESH_VERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texcoord;
};

class CMesh
{
public:
	D3DXVECTOR3 m_pos;
	D3DXVECTOR3 m_size;
	DWORD cant_faces;
	DWORD cant_vertices;
	int cant_indices;
	int bpv;
	char fname[MAX_PATH];
	char mesh_id[255];
	MESH_LAYER layers[MAX_MESH_LAYERS];
	int cant_layers;
	// Internal Data
	MESH_VERTEX *pVertices;
	DWORD *pIndices;
	DWORD *pAttributes;

	CRenderEngine *engine;
	LPDIRECT3DVERTEXBUFFER9 m_vertexBuffer;	
	LPDIRECT3DINDEXBUFFER9 m_indexBuffer;	

	// Construccion y cleanup
	CMesh();
	virtual ~CMesh();
	virtual bool CreateMeshFromData(CRenderEngine *p_engine);		// Crea el mesh pp dicho desde los datos internos
	virtual void ReleaseInternalData();		// Libera los datos internos, una vez que los Buffers del device estan creados no tiene sentido mantenar esos datos del mesh
	virtual void Release();

	// calculos
	virtual bool ComputeBoundingBox();
	virtual void CalcularMatriz(D3DXVECTOR3 pos , D3DXVECTOR3 size , D3DXVECTOR3 rot, D3DXMATRIX *matWorld);
	virtual D3DXVECTOR3 pos_vertice(int i) { return pVertices!=NULL ? pVertices[i].position : D3DXVECTOR3(0,0,0);};			// Abstraccion de las posiciones
	virtual bool hay_internal_data() { return pVertices!=NULL ? true : false;};

	// Relacion con Shaders
	virtual void SetVertexDeclaration();
	virtual void SetShaders();

	// Render
	virtual void Draw();
	virtual void DrawSubset(int i);
	virtual void Render(D3DXVECTOR3 pos, D3DXVECTOR3 size, D3DXVECTOR3 rot=D3DXVECTOR3(0,0,0));
	virtual void Render() { Render(m_pos + m_size*0.5f , m_size);};

	// load
	virtual bool LoadDataFromFile(char *filename);		// Carga los datos del mesh, esto es independiente del device
	virtual bool LoadFromFile(CRenderEngine *p_engine,char *filename,bool keepData=false);
	virtual bool LoadFromXMLFile(CRenderEngine *p_engine,char *filename,char *mesh_name=NULL,int mesh_mat_id=-1);
	

	// interface de creacion de objetos
	virtual void CreateTri(D3DXVECTOR3 a, D3DXVECTOR3 b,D3DXVECTOR3 c);
	virtual void CreateGrid(D3DXVECTOR3 pos,float dx,float dz,int c,int f,char *texture_name,char *normal_map_name="");

};



