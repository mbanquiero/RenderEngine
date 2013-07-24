#pragma once
#pragma pack(1)
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
} MESH_LAYER;

#define MAX_MESH_LAYERS			256


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
	unsigned long cant_faces;
	unsigned long cant_vertices;
	int bpv;
	unsigned long start_vertex;
	unsigned long start_index;
	char fname[MAX_PATH];
	char mesh_id[255];
	MESH_LAYER layers[MAX_MESH_LAYERS];
	int cant_layers;

	CDevice *device;
	CRenderEngine *engine;

	// Data auxiliar para el xml parser
	int *coordinatesIdx;
	int *textCoordsIdx;
	int *matIds;
	FLOAT *vertices;
	FLOAT *normals;
	FLOAT *texCoords;
	int xml_current_tag;
	int xml_current_layer;
	bool xml_multimaterial;
	char path_texturas[MAX_PATH];
	
	CBaseMesh();
	virtual ~CBaseMesh() = 0;
	virtual void Draw() = 0;
	virtual void SetVertexDeclaration() = 0;
	virtual void SetShaders() = 0;

	virtual bool LoadFromXMLFile(CRenderEngine *p_engine,CDevice *p_device,char *filename,char *mesh_name=NULL);
	virtual char ParserXMLLine(char *buffer);
	virtual bool CreateFromXMLData() = 0;


};




class CDX11Mesh : public CBaseMesh
{
public:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;


	CDX11Mesh();
	virtual ~CDX11Mesh();

	virtual bool LoadFromFile(CRenderEngine *p_engine,CDX11Device *p_device,char *filename);
	virtual bool CreateFromXMLData();
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

	virtual void Release();
	virtual bool LoadFromFile(CRenderEngine *p_engine,CDX9Device *p_device,char *filename);
	virtual bool CreateFromXMLData();
	virtual void SetVertexDeclaration();
	virtual void SetShaders();

	virtual void Draw();
	virtual void DrawSubset(int i);

};



