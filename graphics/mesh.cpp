#include "stdafx.h"
#include "/dev/graphics/mesh.h"
#include "/dev/graphics/RenderEngine.h"
#include "/dev/graphics/TGCViewer.h"
#include "/dev/graphics/xstring.h"

#define BUFFER_SIZE  600000

CMesh::CMesh()
{
	cant_faces = 0;
	strcpy(fname,"");
	strcpy(mesh_id,"");
	engine = NULL;
	bpv = sizeof(MESH_VERTEX);					// valor por defecto del stride para mesh comunes
	cant_layers = 0;
	memset(layers,0,sizeof(layers));
	pVertices = NULL;
	pIndices = NULL;
	// dx9
	m_vertexBuffer = NULL;
	m_indexBuffer = NULL;

}

CMesh::~CMesh()
{
	ReleaseInternalData();
	Release();
}

// Libera los datos internos, una vez que los Buffers del device estan creados no tiene sentido mantenar esos datos del mesh
void CMesh::ReleaseInternalData()
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


void CMesh::Release()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
}

void CMesh::CalcularMatriz(D3DXVECTOR3 pos , D3DXVECTOR3 size , D3DXVECTOR3 rot, D3DXMATRIX *matWorld)
{
	D3DXMatrixIdentity(matWorld);

	// determino la escala del objeto, (para que el mesh ocupe exactamente el tamaño del objeto)
	// si tiene cero el size del objeto, usa el del mesh
	float sx = m_size.x;
	float sy = m_size.y;
	float sz = m_size.z;
	float kx = sx && size.x?size.x/sx:1;
	float ky = sy && size.y?size.y/sy:1;
	float kz = sz && size.z?size.z/sz:1;

	// 1- lo llevo al cero en el espacio del objeto pp dicho
	// La traslacion T0 hace que el centro del objeto quede en el cero
	// size = tamaño y P0 punto inicial, en coordenadas del objeto. 
	D3DXMATRIXA16 T0;
	D3DXMatrixTranslation(&T0,-m_pos.x-sx/2,-m_pos.y-sy/2,-m_pos.z-sz/2);
	D3DXMatrixMultiply(matWorld,matWorld,&T0);

	// 2- lo escalo
	D3DXMATRIXA16 Es;
	D3DXMatrixScaling(&Es,kx,ky,kz);
	D3DXMatrixMultiply(matWorld,matWorld,&Es);

	// 3- lo roto
	D3DXMATRIXA16 Rx,Ry,Rz;
	D3DXMatrixRotationZ(&Rz,rot.z);
	D3DXMatrixRotationY(&Ry,rot.y);
	D3DXMatrixRotationX(&Rx,rot.x);
	D3DXMatrixMultiply(matWorld,matWorld,&Rx);
	D3DXMatrixMultiply(matWorld,matWorld,&Ry);
	D3DXMatrixMultiply(matWorld,matWorld,&Rz);

	// 4- Lo traslado a la posicion final
	D3DXMATRIXA16 T;
	D3DXMatrixTranslation(&T,pos.x,pos.y,pos.z);
	D3DXMatrixMultiply(matWorld,matWorld,&T);

}



// Carga internal data con los datos de un archivo Lepton .Y 
// Solo carga estructuras internas, no genera ningun mesh. Para generar el mesh hay que llamar a LoadFromFile
// y ahi se necesita un device y un engine concretos
bool CMesh::LoadDataFromFile(char *filename)
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


bool CMesh::ComputeBoundingBox()
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


bool CMesh::LoadFromFile(CRenderEngine *p_engine,char *filename,bool keepData)
{
	// Uso la clase base para cargar los datos internos (son los mismos sea cual fuera la implementacion grafica)
	// por eso LoadDataFromFile no requiere ni engine ni device
	if(!LoadDataFromFile(filename))
		return false;

	// Creo el mesh pp dicho
	if(!CreateMeshFromData(p_engine))
		return false;

	if(!keepData)
		// una vez creada el mesh, los vertices, indices y attributes, en principio no sirven para una mierda
		// pero le doy la chance de mantenerlos en memoria para casos muy especificos como el software renderer
			ReleaseInternalData();

	// Actualizo otros datos internos
	engine = p_engine;
	strcpy(fname,filename);
	return true;
}

// Carga un mesh desde el formato xml del TGC viewer. 
// Si el xml tiene una escena completa carga solo el primer mesh, o bien el mesh con el nombre que le paso como parametro
bool CMesh::LoadFromXMLFile(CRenderEngine *p_engine,char *filename,char *mesh_name,int mesh_mat_id)
{
	CTGCMeshParser loader;
	if(!loader.LoadMesh(this,filename,mesh_name,mesh_mat_id))
		return false;

	if(!CreateMeshFromData(p_engine))
		return false;
	
	ComputeBoundingBox();
	//TODO: keep data support
	//if(!keepData)
		// una vez creada el mesh, los vertices, indices y attributes, en principio no sirven para una mierda
			// pero le doy la chance de mantenerlos en memoria para casos muy especificos como el software renderer
	ReleaseInternalData();

	// Actualizo otros datos internos
	engine = p_engine;
	strcpy(fname,filename);
	return true;
}


bool CMesh::CreateMeshFromData(CRenderEngine *p_engine)
{
	// Cargo las distintas texturas en el engine, y asocio el nro de textura en el layer del mesh
	for(int i=0;i<cant_layers;++i)
	{
		// Cargo la textura en el pool (o obtengo el nro de textura si es que ya estaba)
		layers[i].nro_textura = p_engine->LoadTexture(layers[i].texture_name);
	}
	
	LPDIRECT3DDEVICE9 g_pd3dDevice = p_engine->g_pd3dDevice;
	// create the vertex buffer
	UINT size = sizeof(MESH_VERTEX) * cant_vertices;
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( size, 0 , 0 , D3DPOOL_DEFAULT, &m_vertexBuffer, NULL ) ) )
		return false;

	MESH_VERTEX *p_gpu_vb;
	if( FAILED( m_vertexBuffer->Lock( 0, size, (void**)&p_gpu_vb, 0 ) ) )
		return false;
	memcpy(p_gpu_vb,pVertices,size);
	m_vertexBuffer->Unlock();


	// Index buffer
	size = sizeof(unsigned long) * cant_indices;
	if( FAILED( g_pd3dDevice->CreateIndexBuffer( size, 0 ,D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_indexBuffer, NULL ) ) )
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



void CMesh::Render(D3DXVECTOR3 pos , D3DXVECTOR3 size , D3DXVECTOR3 rot)
{
	// Computo y seteo la matriz de world asociada al mesh
	CalcularMatriz(pos,size,rot,&engine->m_World);
	// Y actualizo los parametros del shader
	engine->SetShaderTransform();
	// Ahora si dibujo la malla pp dicha
	Draw();
}


// Vertex declaration
void CMesh::SetVertexDeclaration()
{
	engine->g_pd3dDevice->SetVertexDeclaration(engine->m_pVertexDeclaration);
	bpv = sizeof(MESH_VERTEX);
}

void CMesh::SetShaders()
{
	LPDIRECT3DDEVICE9 g_pd3dDevice = engine->g_pd3dDevice;
	engine->g_pEffect = engine->g_pEffectStandard;
	engine->g_pEffect->SetTechnique("RenderScene");
}


void CMesh::Draw()
{
	LPDIRECT3DDEVICE9 g_pd3dDevice = engine->g_pd3dDevice;

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

void CMesh::DrawSubset(int i)
{
	LPDIRECT3DDEVICE9 g_pd3dDevice = engine->g_pd3dDevice;
	ID3DXEffect *g_pEffect = engine->g_pEffect;
	// Hay que setear el material 
	// Set shader texture resource in the pixel shader.
	if(layers[i].nro_textura>=0 && layers[i].nro_textura<engine->cant_texturas)
	{
		g_pEffect->SetTexture("g_Texture", engine->m_texture[layers[i].nro_textura]->g_pTexture);
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

// ---------------------------------------------
void CMesh::CreateTri(D3DXVECTOR3 a, D3DXVECTOR3 b,D3DXVECTOR3 c)
{
	cant_faces = 1;
	cant_indices = cant_vertices = 3;
	pVertices = new MESH_VERTEX[cant_vertices];
	pVertices[0].position = a;
	pVertices[1].position = b;
	pVertices[2].position = c;
	D3DXVECTOR3 v  = b-a;
	D3DXVECTOR3 w  = c-a;
	D3DXVECTOR3 n;
	D3DXVec3Cross(&n,&v,&w);
	D3DXVec3Normalize(&n,&n);
	pVertices[0].normal = n;
	pVertices[1].normal = n;
	pVertices[2].normal = n;

	pIndices = new DWORD[cant_indices];
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;

	pAttributes = new DWORD[cant_faces];
	memset(pAttributes,0,sizeof(DWORD)*cant_faces);
	cant_layers = 1;

	CreateMeshFromData(engine);

}


void CMesh::CreateGrid(D3DXVECTOR3 pos,float dx,float dz,int c,int f,char *texture_name,char *normal_map_name)
{
	cant_vertices = (c+1)*(f+1);
	cant_faces = 2 * f * c;
	cant_indices = 3 * cant_faces;
	float fStepX = dx / (float)c;
	float fStepZ = dz / (float)f;

	pVertices = new MESH_VERTEX[cant_vertices];
	MESH_VERTEX *v = &pVertices[0];

	for ( int i=0; i<=f; ++i )
	{
		for ( int j=0; j<=c ;++j )
		{
			v->position.x = pos.x + -dx/2.0f + j*fStepX;
			v->position.y = pos.y + 0.0f;
			v->position.z = pos.z + dz/2.0f - i*fStepZ;
			v->texcoord.x = ( (float)j / (float)c);
			v->texcoord.y = ( (float)i / (float)f);
			v->normal.x = 0;
			v->normal.y = 1;
			v->normal.z = 0;
			v++;
		}
	}


	pIndices = new DWORD [cant_indices];
	DWORD*    pIndex = &pIndices[0];

	// Fill index buffer
	for ( int i=0; i<f; ++i )
	{
		for ( int j=0; j<c ;++j )
		{
			*pIndex++ = (DWORD)(   i     * ( c+1 ) + j );
			*pIndex++ = (DWORD)(   i     * ( c+1 ) + j + 1 );
			*pIndex++ = (DWORD)( ( i+1 ) * ( c+1 ) + j );

			*pIndex++ = (DWORD)( ( i+1 ) * ( c+1 ) + j );
			*pIndex++ = (DWORD)(   i     * ( c+1 ) + j + 1 );
			*pIndex++ = (DWORD)( ( i+1 ) * ( c+1 ) + j + 1 );
			
		}
	}

	// armo una estructura de layer con un unico layer asociado a la textura  
	pAttributes = new DWORD[cant_faces];
	memset(pAttributes,0,sizeof(DWORD)*cant_faces);
	// diffuse map
	strcpy(layers[0].texture_name , texture_name);
	// normal height map
	strcpy(layers[0].normal_heightmap_name, normal_map_name);
	// atributos del material
	layers[0].Diffuse.a = layers[0].Ambient.a = 1;
	layers[0].Diffuse.r = layers[0].Ambient.r = 1;
	layers[0].Diffuse.g = layers[0].Ambient.g = 1;
	layers[0].Diffuse.b = layers[0].Ambient.b = 1;
	layers[0].ke = 0.4;
	layers[0].kt = 0;
	layers[0].kr = 0;
	cant_layers = 1;

	CreateMeshFromData(engine);
	ComputeBoundingBox();
}

