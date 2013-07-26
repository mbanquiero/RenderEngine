#include "stdafx.h"
#include "/dev/graphics/mesh.h"
#include "/dev/graphics/device.h"
#include "/dev/graphics/dx11device.h"
#include "/dev/graphics/dx9device.h"
#include "/dev/graphics/RenderEngine.h"
#include "/dev/graphics/TGCViewer.h"

#define BUFFER_SIZE  600000

CBaseMesh::CBaseMesh()
{
	cant_faces = 0;
	start_index = 0;
	strcpy(fname,"");
	strcpy(mesh_id,"");
	device = NULL;
	engine = NULL;
	bpv = sizeof(MESH_VERTEX);					// valor por defecto del stride para mesh comunes
	cant_layers = 0;
	memset(layers,0,sizeof(layers));
	pVertices = NULL;
	pIndices = NULL;

}

CBaseMesh::~CBaseMesh()
{
	ReleaseInternalData();
}

// Libera los datos internos, una vez que los Buffers del device estan creados no tiene sentido mantenar esos datos del mesh
void CBaseMesh::ReleaseInternalData()
{
	if(pVertices!=NULL)
	{
		delete []pVertices;
		pVertices = NULL;
	}

	if(pIndices!=NULL)
	{
		delete []pIndices;
		pIndices = NULL;
	}

	if(pAttributes!=NULL)
	{
		delete []pAttributes;
		pAttributes = NULL;
	}
}


// Carga internal data con los datos de un archivo Lepton .Y 
// Solo carga estructuras internas, no genera ningun mesh. Para generar el mesh hay que llamar a LoadFromFile
// y ahi se necesita un device y un engine concretos
bool CBaseMesh::LoadDataFromFile(char *filename)
{
	FILE *fp = fopen(filename,"rb");
	if(fp==NULL)
		return false;

	int header[9];
	// leo el header
	fread(&header,sizeof(header),1,fp);
	int version = header[6];

	// Cantidad de layers
	DWORD g_dwNumMaterials;
	fread(&g_dwNumMaterials,sizeof(g_dwNumMaterials),1,fp);
	cant_layers = g_dwNumMaterials;

	for(int i=0;i<g_dwNumMaterials;++i)
	{
		// nombre de la textura
		char texture_name[256];
		memset(texture_name,0,sizeof(texture_name));
		fread(texture_name,sizeof(texture_name),1,fp);

		// Material del layer
		D3DMATERIAL9 g_pMeshMaterials;
		fread(&g_pMeshMaterials,sizeof(D3DMATERIAL9),1,fp);

		// Abmient y diffuse se usan tal cual (para ambient y diffuse respectivamente)
		// Pero specular y emissive (que no tiene soporte fixed), estan hardcodeados para usar asi:
		// Specular.R = luz specular pp dicha
		// Specular.G, Specular.B -> no usados, se supone que estan en cero
		// Emissive.r = Reflexion
		// Emissive.g = Transparencia
		// Emissive.b = no usado
		// Emissive.b = no usado
		// el solid usa esos campos para almacenar info de transparencia y reflexion no
		// soportada en forma nativa por el D3DMATERIAL del Directx

		layers[i].Ambient = g_pMeshMaterials.Ambient;
		layers[i].Diffuse = g_pMeshMaterials.Diffuse;
		layers[i].ke = g_pMeshMaterials.Specular.b;
		layers[i].kr = g_pMeshMaterials.Emissive.r;
		layers[i].kt = g_pMeshMaterials.Emissive.g;
		strcpy(layers[i].texture_name,texture_name);
		// Cargo la textura en el pool (o obtengo el nro de textura si es que ya estaba)
		//layers[i].nro_textura = p_engine->LoadTexture(texture_name);
	}

	// Cantidad de caras
	fread(&cant_faces,sizeof(cant_faces),1,fp);
	// Cantidad de vertices
	fread(&cant_vertices,sizeof(cant_vertices),1,fp);
	// Bytes por vertice (usualmente bps = 32 bytes)
	fread(&bpv,sizeof(bpv),1,fp);
	// Vertex buffer
	pVertices = new MESH_VERTEX[cant_vertices];
	fread(pVertices,bpv,cant_vertices,fp);
	// Transformo de lepton format a dx format
	for(int i=0;i<cant_vertices;++i)
	{
		float x = pVertices[i].position.x;
		float y = pVertices[i].position.y;
		float z = pVertices[i].position.z;
		pVertices[i].position.x = y;
		pVertices[i].position.y = z;
		pVertices[i].position.z = x;

		float nx = pVertices[i].normal.x;
		float ny = pVertices[i].normal.y;
		float nz = pVertices[i].normal.z;
		pVertices[i].normal.x = ny;
		pVertices[i].normal.y = nz;
		pVertices[i].normal.z = nx;
	}

	cant_indices = cant_faces*3;
	pIndices = new DWORD[cant_indices];
	// Index buffer
	fread(pIndices,cant_indices*sizeof(DWORD),1,fp);
	// nro de layer por face (attributtes)
	pAttributes = new DWORD[cant_faces];
	fread(pAttributes,cant_faces*sizeof(DWORD),1,fp);
	fclose(fp);

	// Aprovecho para computar el tamaño y la posicion del mesh
	ComputeBoundingBox();

	return true;
}


bool CBaseMesh::ComputeBoundingBox()
{
	if(!cant_vertices || !hay_internal_data())
		return false;

	D3DXVECTOR3 min = D3DXVECTOR3 (10000,10000,10000);
	D3DXVECTOR3 max = D3DXVECTOR3 (-10000,-10000,-10000);
	for(int i=0;i<cant_vertices;++i)
	{
		D3DXVECTOR3 pos = pos_vertice(i);
		if(pos.x<min.x)
			min.x = pos.x;
		if(pos.y<min.y)
			min.y = pos.y;
		if(pos.z<min.z)
			min.z = pos.z;

		if(pos.x>max.x)
			max.x = pos.x;
		if(pos.y>max.y)
			max.y = pos.y;
		if(pos.z>max.z)
			max.z = pos.z;
	}

	m_pos = min;
	m_size = max-min;
	return true;
}


bool CBaseMesh::LoadFromFile(CRenderEngine *p_engine,CDevice *p_device,char *filename,bool keepData)
{
	// Uso la clase base para cargar los datos internos (son los mismos sea cual fuera la implementacion grafica)
	// por eso LoadDataFromFile no requiere ni engine ni device
	if(!LoadDataFromFile(filename))
		return false;

	// Creo el mesh pp dicho
	if(!CreateMeshFromData(p_engine,(CDX11Device *)p_device))
		return false;

	if(!keepData)
		// una vez creada el mesh, los vertices, indices y attributes, en principio no sirven para una mierda
		// pero le doy la chance de mantenerlos en memoria para casos muy especificos como el software renderer
			ReleaseInternalData();

	// Actualizo otros datos internos
	device = p_device;
	engine = p_engine;
	strcpy(fname,filename);
	return true;
}

// Carga un mesh desde el formato xml del TGC viewer. 
// Si el xml tiene una escena completa carga solo el primer mesh, o bien el mesh con el nombre que le paso como parametro
bool CBaseMesh::LoadFromXMLFile(CRenderEngine *p_engine,CDevice *p_device,char *filename,char *mesh_name,int mesh_mat_id)
{
	CTGCMeshParser loader;
	if(!loader.LoadMesh(this,filename,mesh_name,mesh_mat_id))
		return false;

	if(!CreateMeshFromData(p_engine,(CDX11Device *)p_device))
		return false;
	
	ComputeBoundingBox();
	//TODO: keep data support
	//if(!keepData)
		// una vez creada el mesh, los vertices, indices y attributes, en principio no sirven para una mierda
			// pero le doy la chance de mantenerlos en memoria para casos muy especificos como el software renderer
	ReleaseInternalData();

	// Actualizo otros datos internos
	device = p_device;
	engine = p_engine;
	strcpy(fname,filename);

	return true;
}


// -------------------------------------------------------------------------------------------------
CDX11Mesh::CDX11Mesh()
{
	cant_faces = 0;
	start_index = 0;
	strcpy(fname,"");
	device = NULL;
	m_vertexBuffer = NULL;
	m_indexBuffer = NULL;
}

CDX11Mesh::~CDX11Mesh()
{
	Release();
}

void CDX11Mesh::Release()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
}



bool CDX11Mesh::CreateMeshFromData(CRenderEngine *p_engine,CDevice *p_device)
{
	// Ahora con los datos del mesh cargo una mesh de directx 11
	CDX11Device *p_d11device = (CDX11Device *)p_device;
	// Cargo las distintas texturas en el engine, y asocio el nro de textura en el layer del mesh
	for(int i=0;i<cant_layers;++i)
	{
		// Cargo la textura en el pool (o obtengo el nro de textura si es que ya estaba)
		layers[i].nro_textura = p_engine->LoadTexture(layers[i].texture_name);
	}

	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(MESH_VERTEX) * cant_vertices;             
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
	p_d11device->dev->CreateBuffer(&bd, NULL, &m_vertexBuffer);       // create the buffer

	// copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	p_d11device->devcon->Map(m_vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, pVertices, bd.ByteWidth);											// copy the data
	p_d11device->devcon->Unmap(m_vertexBuffer, NULL);                                      // unmap the buffer

	
	// Index buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(unsigned long) * cant_indices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;       // use as a index buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
	p_d11device->dev->CreateBuffer(&bd, NULL, &m_indexBuffer);       // create the buffer

	// El index buffer es mas complicado, porque tengo que dividir por layers. 
	int index = 0;
	DWORD *pIndicesAux = new DWORD[cant_indices];
	for(int i=0;i<cant_layers;++i)
	{
		// Estoy trabajando con el layer i, busco en la tabla de atributos las caras que pertencen al layer i
		layers[i].start_index = index;
		for(int j=0;j<cant_faces;++j)
			if(pAttributes[j]==i)
			{
				// Agrego esta cara al subset
				pIndicesAux[index++] = pIndices[j*3];
				pIndicesAux[index++] = pIndices[j*3+1];
				pIndicesAux[index++] = pIndices[j*3+2];
			}
		layers[i].cant_indices = index-layers[i].start_index;
		// Paso al siguiente layer
	}
	// Se supone que index==cant_indices
	// Ahora si copio desde el indice auxiliar, que esta ordenado por subset.
	p_d11device->devcon->Map(m_indexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);   
	memcpy(ms.pData, pIndicesAux, bd.ByteWidth);								
	p_d11device->devcon->Unmap(m_indexBuffer, NULL);                                   

	delete []pIndicesAux;

	return true;
}

void CDX11Mesh::SetVertexDeclaration()
{
	CDX11Device *p_device = (CDX11Device *) device;
	p_device->devcon->IASetInputLayout(p_device->pLayout);
	bpv = sizeof(MESH_VERTEX);

}


// set the shader objects
void CDX11Mesh::SetShaders()
{
	CDX11Device *p_device = (CDX11Device *) device;
	p_device->devcon->VSSetShader(p_device->pVS, 0, 0);
	p_device->devcon->PSSetShader(p_device->pPS, 0, 0);
}

void CDX11Mesh::Draw()
{
	UINT stride = bpv;
	UINT offset = 0;
	CDX11Device *p_device = (CDX11Device *)device;

	// Seteo el index y el vertex buffer
	p_device->devcon->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	p_device->devcon->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// select which primtive type we are using
	p_device->devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Seteo el vertex declaration
	SetVertexDeclaration();
	// Seteo los shaders
	SetShaders();

	// dibujo cada subset
	for(int i=0;i<cant_layers;++i)
		DrawSubset(i);
}

void CDX11Mesh::DrawSubset(int i)
{
	// Hay que setear el material 

	// Set shader texture resource in the pixel shader.
	CDX11Device *p_device = (CDX11Device *)device;
	if(layers[i].nro_textura>=0 && layers[i].nro_textura<engine->cant_texturas)
	{
		CDX11Texture *p_texture = (CDX11Texture *)engine->m_texture[layers[i].nro_textura];
		p_device->devcon->PSSetShaderResources(0, 1, &p_texture->resourceView);
	}

	// por fin dibujo el subset pp dicho
	p_device->devcon->DrawIndexed(layers[i].cant_indices,layers[i].start_index, 0);

}







// -------------------------------------------------------------------------------------------------
// soporte para DX 9
// -------------------------------------------------------------------------------------------------
CDX9Mesh::CDX9Mesh() : CBaseMesh()
{
	m_vertexBuffer = NULL;
	m_indexBuffer = NULL;
}

CDX9Mesh::~CDX9Mesh()
{
	Release();
}

void CDX9Mesh::Release()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
}


bool CDX9Mesh::CreateMeshFromData(CRenderEngine *p_engine,CDevice *p_device)
{
	CDX9Device *p_d9device =  (CDX9Device *)p_device;

	// Cargo las distintas texturas en el engine, y asocio el nro de textura en el layer del mesh
	for(int i=0;i<cant_layers;++i)
	{
		// Cargo la textura en el pool (o obtengo el nro de textura si es que ya estaba)
		layers[i].nro_textura = p_engine->LoadTexture(layers[i].texture_name);
	}
	
	// create the vertex buffer
	UINT size = sizeof(MESH_VERTEX) * cant_vertices;
	if( FAILED( p_d9device->g_pd3dDevice->CreateVertexBuffer( size, 0 , 
		0 , D3DPOOL_DEFAULT, &m_vertexBuffer, NULL ) ) )
		return false;

	MESH_VERTEX *p_gpu_vb;
	if( FAILED( m_vertexBuffer->Lock( 0, size, (void**)&p_gpu_vb, 0 ) ) )
		return false;
	memcpy(p_gpu_vb,pVertices,size);
	m_vertexBuffer->Unlock();


	// Index buffer
	size = sizeof(unsigned long) * cant_indices;
	if( FAILED( p_d9device->g_pd3dDevice->CreateIndexBuffer( size, 0 ,D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_indexBuffer, NULL ) ) )
		return false;

	// El index buffer es mas complicado, porque tengo que dividir por layers. 
	DWORD *pIndicesAux = new DWORD[cant_indices];

	int index = 0;
	for(int i=0;i<cant_layers;++i)
	{
		// Estoy trabajando con el layer i, busco en la tabla de atributos las caras que pertencen al layer i
		layers[i].start_index = index;
		for(int j=0;j<cant_faces;++j)
			if(pAttributes[j]==i)
			{
				// Agrego esta cara al subset
				pIndicesAux[index++] = pIndices[j*3];
				pIndicesAux[index++] = pIndices[j*3+1];
				pIndicesAux[index++] = pIndices[j*3+2];
			}
			layers[i].cant_indices = index-layers[i].start_index;
			// Paso al siguiente layer
	}
	// Se supone que index==cant_indices
	// Ahora si copio desde el indice auxiliar, que esta ordenado por subset.

	unsigned long *p_gpu_ib;
	if( FAILED( m_indexBuffer->Lock( 0, size, (void**)&p_gpu_ib, 0 ) ) )
		return false;
	memcpy(p_gpu_ib,pIndicesAux,size);
	m_indexBuffer->Unlock();

	delete []pIndicesAux;

	return true;
}




// Vertex declaration
void CDX9Mesh::SetVertexDeclaration()
{
	CDX9Device *p_device = (CDX9Device *)device;
	LPDIRECT3DDEVICE9 g_pd3dDevice = p_device->g_pd3dDevice;
	g_pd3dDevice->SetVertexDeclaration(p_device->m_pVertexDeclaration);
	bpv = sizeof(MESH_VERTEX);
}

void CDX9Mesh::SetShaders()
{
	CDX9Device *p_device = (CDX9Device *)device;
	LPDIRECT3DDEVICE9 g_pd3dDevice = p_device->g_pd3dDevice;
	p_device->g_pEffect = p_device->g_pEffectStandard;
	p_device->g_pEffect->SetTechnique("RenderScene");
}


void CDX9Mesh::Draw()
{
	LPDIRECT3DDEVICE9 g_pd3dDevice = ((CDX9Device *)device)->g_pd3dDevice;

	// Seteo el index y el vertex buffer
	g_pd3dDevice->SetStreamSource( 0, m_vertexBuffer, 0, sizeof(MESH_VERTEX));
	g_pd3dDevice->SetIndices(m_indexBuffer);

	// Seteo el vertex declaration
	SetVertexDeclaration();
	// Seteo los shaders (effect tecnica)
	SetShaders();

	// dibujo cada subset
	for(int i=0;i<cant_layers;++i)
		DrawSubset(i);
}

void CDX9Mesh::DrawSubset(int i)
{
	CDX9Device *p_device = (CDX9Device *)device;
	LPDIRECT3DDEVICE9 g_pd3dDevice = p_device->g_pd3dDevice;
	ID3DXEffect *g_pEffect = p_device->g_pEffect;
	// Hay que setear el material 
	// Set shader texture resource in the pixel shader.
	if(layers[i].nro_textura>=0 && layers[i].nro_textura<engine->cant_texturas)
	{
		CDX9Texture *p_texture = (CDX9Texture *)engine->m_texture[layers[i].nro_textura];
		p_device->g_pEffect->SetTexture("g_Texture", p_texture->g_pTexture);
	}

	// por fin dibujo el subset pp dicho
	UINT cPasses;
	g_pEffect->Begin(&cPasses, 0);
	for(UINT pass = 0;pass<cPasses;++pass)
	{
		g_pEffect->BeginPass(pass);
		g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,cant_vertices,layers[i].start_index,layers[i].cant_indices/3);
		g_pEffect->EndPass();
	}
	g_pEffect->End();

}



