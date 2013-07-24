#include "stdafx.h"
#include "/dev/graphics/mesh.h"
#include "/dev/graphics/device.h"
#include "/dev/graphics/dx11device.h"
#include "/dev/graphics/dx9device.h"
#include "/dev/graphics/RenderEngine.h"

#define BUFFER_SIZE  600000
D3DCOLORVALUE ParserXMLColor(char *buffer);
bool ParserIntStream(char *buffer,int *S,int count);
bool ParserFloatStream(char *buffer,float *S,int count);

CBaseMesh::CBaseMesh()
{
	cant_faces = 0;
	start_index = 0;
	strcpy(fname,"");
	strcpy(mesh_id,"");
	device = NULL;
	bpv = sizeof(MESH_VERTEX);					// valor por defecto del stride para mesh comunes
	cant_layers = 0;
}

CBaseMesh::~CBaseMesh()
{

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


bool CDX11Mesh::LoadFromFile(CRenderEngine *p_engine,CDX11Device *p_device,char *filename)
{
	FILE *fp = fopen(filename,"rb");
	if(fp==NULL)
		return false;

	int header[9];
	// leo el header
	fread(&header,sizeof(header),1,fp);
	int version = header[6];

	// Cantidad de layers
	unsigned long g_dwNumMaterials;
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

		// Cargo la textura en el pool (o obtengo el nro de textura si es que ya estaba)
		layers[i].nro_textura = p_engine->LoadTexture(texture_name);
	}


	// Cantidad de caras
	fread(&cant_faces,sizeof(cant_faces),1,fp);
	// Cantidad de vertices
	fread(&cant_vertices,sizeof(cant_vertices),1,fp);
	// Bytes por vertice (usualmente bps = 32 bytes)
	fread(&bpv,sizeof(bpv),1,fp);
	// Vertex buffer
	MESH_VERTEX *pVertices = new MESH_VERTEX[cant_vertices];
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

	int cant_indices = cant_faces*3;
	unsigned long  *pIndices = new unsigned long[cant_indices];
	// Index buffer
	fread(pIndices,cant_indices*sizeof(unsigned long),1,fp);
	// Attributos
	unsigned long  *pAttributes = new unsigned long[cant_faces];
	fread(pAttributes,cant_faces*sizeof(unsigned long),1,fp);
	fclose(fp);


	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(MESH_VERTEX) * cant_vertices;             
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
	p_device->dev->CreateBuffer(&bd, NULL, &m_vertexBuffer);       // create the buffer

	// copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	p_device->devcon->Map(m_vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, pVertices, bd.ByteWidth);											// copy the data
	p_device->devcon->Unmap(m_vertexBuffer, NULL);                                      // unmap the buffer

	
	// Index buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(unsigned long) * cant_indices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;       // use as a index buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
	p_device->dev->CreateBuffer(&bd, NULL, &m_indexBuffer);       // create the buffer

	// El index buffer es mas complicado, porque tengo que dividir por layers. 
	unsigned long  *pIndicesAux = new unsigned long[cant_indices];

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
	p_device->devcon->Map(m_indexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);   
	memcpy(ms.pData, pIndicesAux, bd.ByteWidth);								
	p_device->devcon->Unmap(m_indexBuffer, NULL);                                   


	// Aprovecho para computar el tamaño y la posicion del mesh
	D3DXVECTOR3 min = D3DXVECTOR3 (10000,10000,10000);
	D3DXVECTOR3 max = D3DXVECTOR3 (-10000,-10000,-10000);
	for(int i=0;i<cant_vertices;++i)
	{
		if(pVertices[i].position.x<min.x)
			min.x = pVertices[i].position.x;
		if(pVertices[i].position.y<min.y)
			min.y = pVertices[i].position.y;
		if(pVertices[i].position.z<min.z)
			min.z = pVertices[i].position.z;

		if(pVertices[i].position.x>max.x)
			max.x = pVertices[i].position.x;
		if(pVertices[i].position.y>max.y)
			max.y = pVertices[i].position.y;
		if(pVertices[i].position.z>max.z)
			max.z = pVertices[i].position.z;
	}

	m_pos = min;
	m_size = max-min;

	// libero la memoria de los vertices, indices y attributos
	delete pVertices;
	delete pIndices;
	delete pIndicesAux;
	delete pAttributes;
	
	// Actualizo datos internos
	device = p_device;
	engine = p_engine;
	strcpy(fname,filename);

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



// Carga un mesh desde el formato xml del TGC viewer. 
// Si el xml tiene una escena completa carga solo el primer mesh, o bien el mesh con el nombre que le paso como parametro
bool CBaseMesh::LoadFromXMLFile(CRenderEngine *p_engine,CDevice *p_device,char *filename,char *mesh_name)
{
	FILE *fp = fopen(filename,"rt");
	if(fp==NULL)
		return false;

	// Path de texturas x defecto formato xml
	strcpy(path_texturas,filename);
	char *s = strrchr(path_texturas,'/');
	if(s!=NULL)
		*s = '\0';

	// Datos internos del basemesh
	device = p_device;
	engine = p_engine;
	strcpy(fname,filename);

	if(mesh_name!=NULL)
		strcpy(mesh_id,mesh_name);
	else
		strcpy(mesh_id,"");

	// Inicializo la Data auxiliar para el xml parser
	matIds = coordinatesIdx = textCoordsIdx = NULL;
	vertices = normals = texCoords = NULL;
	xml_current_tag = xml_current_layer = -1;
	xml_multimaterial = false;

	// Leo y parseo el xml
	char *buffer = new char[BUFFER_SIZE];
	bool terminar = false;
	while(fgets(buffer,BUFFER_SIZE,fp)!=NULL && !terminar)
	{
		ltrim(buffer);
		if(ParserXMLLine(buffer)==-1)
			terminar = true;
	}
	delete buffer,
	fclose(fp);

	// Y ahora si creo la malla pp dicha
	CreateFromXMLData();

	// libero los datos auxiliares
	SAFE_DELETE(coordinatesIdx);
	SAFE_DELETE(textCoordsIdx);
	SAFE_DELETE(matIds);
	SAFE_DELETE(vertices);
	SAFE_DELETE(normals);
	SAFE_DELETE(texCoords);

		// Actualizo datos internos
	cant_faces = cant_vertices / 3;
	layers[0].start_index = 0;
	layers[0].cant_indices = cant_vertices;			// cant_indices == cant_vertices 

	return true;
}

bool CDX11Mesh::CreateFromXMLData()
{
	CDX11Device *p_device = (CDX11Device *)device;
	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(MESH_VERTEX) * cant_vertices;             
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
	p_device->dev->CreateBuffer(&bd, NULL, &m_vertexBuffer);       // create the buffer

	// copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	p_device->devcon->Map(m_vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	// copy the data
	// Aprovecho para computar el tamaño y la posicion del mesh
	D3DXVECTOR3 min = D3DXVECTOR3 (10000,10000,10000);
	D3DXVECTOR3 max = D3DXVECTOR3 (-10000,-10000,-10000);

	MESH_VERTEX *pVertices = (MESH_VERTEX *)ms.pData;
	for(int i=0;i<cant_vertices;++i)
	{
		int index = coordinatesIdx[i] * 3;
		pVertices[i].position.x = vertices[index];
		pVertices[i].position.y = vertices[index + 1];
		pVertices[i].position.z = vertices[index + 2];

		pVertices[i].normal.x = normals[index];
		pVertices[i].normal.y = normals[index + 1];
		pVertices[i].normal.z = normals[index + 2];


		index = textCoordsIdx[i] * 2;
		pVertices[i].texcoord.x = texCoords[index];
		pVertices[i].texcoord.y = texCoords[index + 1];

		if(pVertices[i].position.x<min.x)
			min.x = pVertices[i].position.x;
		if(pVertices[i].position.y<min.y)
			min.y = pVertices[i].position.y;
		if(pVertices[i].position.z<min.z)
			min.z = pVertices[i].position.z;

		if(pVertices[i].position.x>max.x)
			max.x = pVertices[i].position.x;
		if(pVertices[i].position.y>max.y)
			max.y = pVertices[i].position.y;
		if(pVertices[i].position.z>max.z)
			max.z = pVertices[i].position.z;

	}
	p_device->devcon->Unmap(m_vertexBuffer, NULL);                                      // unmap the buffer

	m_pos = min;
	m_size = max-min;

	// Index buffer
	int cant_indices = cant_vertices;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(unsigned long) * cant_indices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;       // use as a index buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer
	p_device->dev->CreateBuffer(&bd, NULL, &m_indexBuffer);       // create the buffer

	p_device->devcon->Map(m_indexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);   
	unsigned long *pIndicesAux = (unsigned long *)ms.pData;

	if(cant_layers==1 || true)
	{
		// index buffer trivial
		for(int i=0;i<cant_indices;++i)
			pIndicesAux[i] = i;
	}
	else
	{
		// Organizo por layer
		int t = 0;
		for(int n = 0 ; n< cant_layers ;++n)
		{
			// inicializo el layer
			layers[n].start_index = t;
			layers[n].cant_indices = 0;
			// Cada 3 indices hay un attributo que indica el nro de layer, un attrib x face
			cant_faces = cant_indices / 3;
			for(int i=0;i<cant_faces;++i)
			{
				int nro_layer = matIds[i];
				if(nro_layer==n)
				{
					pIndicesAux[t++] = i*3;
					pIndicesAux[t++] = i*3+1;
					pIndicesAux[t++] = i*3+2;
					layers[n].cant_indices += 3;
				}
			}
		}
	}
	p_device->devcon->Unmap(m_indexBuffer, NULL);                                   

	return true;
}

// XML parser
// Parsea una linea de xml, devuelve true si la proceso, o false si no reconocido nada, para que el caller pueda 
// seguir procesando, o -1 indicando que tiene que terminar de procesar el archivo por encontrar el final del mesh

// Ojo que el formato es un poco enganñoso. Por un lado estan los vertices, 
// <vertices count='7371'>3.42944 91.7358  
// vienen en formato x0,y0,z0 , x1,y1,z1 , .....
// la cantidad de vertices es vertices_count / 3 
// Sin embargo esos son posiciones de vertices, no vertices en si. 
// La lista de coordenadas, 
// <coordinatesIdx count='14052'>1 11 
// Es parecida PERO NO IGUAL, al index buffer, apunta a las coordenadas de un vertices, pero NO AL VERTICE.
// Notar que no HAY NINGUNA ESTRUCTURA CON LOS DATOS DEL VERTICE. 
// Al vertice hay que armarlo en el momento, y por eso de momento no hay una estructura de indices buena, 
// Pueden quedar vertices repetidos y los indices son siempre correlativos.
// TODO: detectar que hay vertices duplicados, y generar como corresponde un index y vertexbuffer
char CBaseMesh::ParserXMLLine(char *buffer)
{
	char procesada = 0;
	/*if(strncmp(buffer,"<materials count=" , 17)==0)
	{
		// Este tag esta al pedo, siempre el material count es 1 cuando es un solo mesh
		// en las escenas hay varios materiales. Lo que yo llamo material es lo que el mesh llama sub - material
		cant_layers = atoi(buffer + 18);
	}
	else
	*/
	
	if(strncmp(buffer,"<m name=" , 8)==0)
	{

		// Determino si tiene sub material o es un unico material
		if(strstr(buffer,"'Multimaterial'")!= NULL)
			// tiene submateriales, 
				xml_multimaterial = true;
		else
		{
			// Agrego el unico layer
			xml_current_tag = 0;
			xml_current_layer = cant_layers++;
			memset(&layers[xml_current_layer],0,sizeof(MESH_LAYER));
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<subM name=" , 11)==0)
	{
		// se supone que xml_multimaterial == true
		// Agrego un layer de material
		xml_current_tag = 0;
		xml_current_layer = cant_layers++;
		memset(&layers[xml_current_layer],0,sizeof(MESH_LAYER));
		procesada = 1;
	}

	if(strncmp(buffer,"<ambient>" , 9)==0)
	{
		if(xml_current_tag==0 && xml_current_layer>=0 && xml_current_layer<cant_layers)
			layers[xml_current_layer].Ambient = ParserXMLColor(buffer + 10);
		procesada = 1;
	}

	if(strncmp(buffer,"<diffuse>" , 9)==0)
	{
		if(xml_current_tag==0 && xml_current_layer>=0 && xml_current_layer<cant_layers)
			layers[xml_current_layer].Diffuse = ParserXMLColor(buffer + 10);
		procesada = 1;
	}

	if(strncmp(buffer,"<specular>" , 10)==0)
	{
		if(xml_current_tag==0 && xml_current_layer>=0 && xml_current_layer<cant_layers)
		{
			D3DCOLORVALUE Specular = ParserXMLColor(buffer + 11);
			layers[xml_current_layer].ke = Specular.b;
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<opacity>" , 9)==0)
	{
		if(xml_current_tag==0 && xml_current_layer>=0 && xml_current_layer<cant_layers)
			layers[xml_current_layer].kt = atof(buffer + 10);
		procesada = 1;
	}

	if(strncmp(buffer,"<bitmap" , 7)==0)
	{
		// busco el>
		char *p = strchr(buffer,'>');
		if(p!=NULL)
		{
			// busco el <
			char *q = strchr(p,'<');
			if(q!=NULL)
			{
				*q = '\0';
				char texture_name[MAX_PATH];
				sprintf(texture_name,"%s/Textures/%s",path_texturas,p+1);
				if(xml_current_tag==0 && xml_current_layer>=0 && xml_current_layer<cant_layers)
				{
					// Cargo la textura en el pool (o obtengo el nro de textura si es que ya estaba)
					layers[xml_current_layer].nro_textura = engine->LoadTexture(texture_name);
				}
			}
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<mesh name=" , 11)==0)
	{
		// si ya estaba procesando un mesh, y encontro el header de otro, ya puede terminar
		// porque ya cargo lo que precisaba
		if(xml_current_tag==1)
			procesada = -1;		// indico que tiene que terminar de procesar el archivo
		else
		{
			// tomo el nombre del mesh
			char *p = strchr(buffer+12,'\'');
			if(p!=NULL)
			{
				*p = 0;
				// Verifico si coincide con el mesh que quiero cargar, salvo que quiera cargar el primero
				if(esta_vacio(mesh_id))
				{
					// quiere cargar solo el primero, entonces, asigno el id
					strcpy(mesh_id,buffer+12);
					// y pongo el status en cargar mesh (=1)
					xml_current_tag = 1;
				}
				else
				{
					// solo carga el mesh que me pide
					if(strcmp(mesh_id,buffer+12)==0)
						// econtre el mesh que quiere cargar, pongo el status en cargar mesh (=1)
						xml_current_tag = 1;
				}
			}
			procesada = 1;
		}
	}

	if(strncmp(buffer,"<coordinatesIdx count=" , 22)==0)
	{
		if(xml_current_tag==1)
		{
			int count = atoi(buffer + 23);
			if(count>0)
			{
				cant_vertices = count;
				coordinatesIdx = new int[count];
				char *p = strchr(buffer+23,'>');
				if(p!=NULL)
					ParserIntStream(p+1,coordinatesIdx,count);
			}
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<textCoordsIdx count=" , 21)==0)
	{
		if(xml_current_tag==1)
		{
			int count = atoi(buffer + 22);
			if(count>0)
			{
				textCoordsIdx = new int[count];
				char *p = strchr(buffer+22,'>');
				if(p!=NULL)
					ParserIntStream(p+1,textCoordsIdx,count);
			}
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<vertices count=" , 16)==0)
	{
		if(xml_current_tag==1)
		{
			int count = atoi(buffer + 17);
			if(count>0)
			{
				vertices = new FLOAT[count];
				char *p = strchr(buffer+17,'>');
				if(p!=NULL)
					ParserFloatStream(p+1,vertices,count);
			}
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<normals count=" , 15)==0)
	{
		if(xml_current_tag==1)
		{
			int count = atoi(buffer + 16);
			if(count>0)
			{
				normals = new FLOAT[count];
				char *p = strchr(buffer+16,'>');
				if(p!=NULL)
					ParserFloatStream(p+1,normals,count);
			}
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<texCoords count=" , 17)==0)
	{
		if(xml_current_tag==1)
		{
			int count = atoi(buffer + 18);
			if(count>0)
			{
				texCoords = new FLOAT[count];
				char *p = strchr(buffer+18,'>');
				if(p!=NULL)
					ParserFloatStream(p+1,texCoords,count);
			}
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<matIds count=" , 14)==0)
	{
		if(xml_current_tag==1)
		{
			int count = atoi(buffer + 15);
			if(count>0)
			{
				matIds = new int[count];
				char *p = strchr(buffer+18,'>');
				if(p!=NULL)
					ParserIntStream(p+1,matIds,count);
			}
		}
		procesada = 1;
	}

	// Le doy la oportunidad al caller de seguir procesando la linea
	return procesada;
}


// helpers para xml
// [150.0,150.0,150.0,255.0]
D3DCOLORVALUE ParserXMLColor(char *buffer)
{
	int i = 0;
	FLOAT val[4];
	D3DCOLORVALUE color;
	color.a = color.r = color.g = color.b = 0;
	char sep[] = {',',',',',',']'};
	bool error = false;
	for(int f=0;f<4 && !error;++f)
	{
		char saux[40];
		int t = 0;
		while(buffer[i] && buffer[i]!=sep[f])
			saux[t++] = buffer[i++];
		if(buffer[i]!=sep[f])
			error = true;
		else
		{
			saux[t] = '\0';
			val[f] = atof(saux);
			++i;
		}
	}

	if(!error)
	{
		color.r = val[0]/255.0f;
		color.g = val[1]/255.0f;
		color.b = val[2]/255.0f;
		color.a = val[3]/255.0f;
	}

	return color;
}

// [150.0,150.0,150.0]
D3DXVECTOR3 ParserXMLVector3(char *buffer)
{
	int i = 0;
	FLOAT val[3];
	D3DXVECTOR3 vect;
	vect.x = vect.y = vect.z = 0;
	char sep[] = {',' , ','  , ']' };
	bool error = false;
	for(int f=0;f<3 && !error;++f)
	{
		char saux[40];
		int t = 0;
		while(buffer[i] && buffer[i]!=sep[f])
			saux[t++] = buffer[i++];
		if(buffer[i]!=sep[f])
			error = true;
		else
		{
			saux[t] = '\0';
			val[f] = atof(saux);
			++i;
		}
	}

	if(!error)
	{
		vect.x = val[0];
		vect.y = val[1];
		vect.z = val[2];
	}

	return vect;
}

D3DXVECTOR4 ParserXMLVector4(char *buffer)
{
	int i = 0;
	FLOAT val[4];
	D3DXVECTOR4 vect;
	vect.x = vect.y = vect.z = vect.w = 0;
	char sep[] = {',' , ',' , ',' , ']' };
	bool error = false;
	for(int f=0;f<4 && !error;++f)
	{
		char saux[40];
		int t = 0;
		while(buffer[i] && buffer[i]!=sep[f])
			saux[t++] = buffer[i++];
		if(buffer[i]!=sep[f])
			error = true;
		else
		{
			saux[t] = '\0';
			val[f] = atof(saux);
			++i;
		}
	}

	if(!error)
	{
		vect.x = val[0];
		vect.y = val[1];
		vect.z = val[2];
		vect.w = val[3];
	}

	return vect;
}

bool ParserIntStream(char *buffer,int *S,int count)
{
	bool error = false;
	int i = 0;
	int c = 0;
	while(buffer[i] && c<count && !error)
	{
		char saux[40];
		int t = 0;
		while(buffer[i] && c<count && buffer[i]!=32 && !error)
			saux[t++] = buffer[i++];
		if(buffer[i]!=32)
			error = true;
		else
		{
			saux[t] = '\0';
			S[c++] = atoi(saux);
			++i;
		}
	}

	return error;
}


bool ParserFloatStream(char *buffer,float *S,int count)
{
	bool error = false;
	int i = 0;
	int c = 0;
	while(buffer[i] && c<count && !error)
	{
		char saux[40];
		int t = 0;
		while(buffer[i] && c<count && buffer[i]!=32 && !error)
			saux[t++] = buffer[i++];
		if(buffer[i]!=32)
			error = true;
		else
		{
			saux[t] = '\0';
			S[c++] = atof(saux);
			++i;
		}
	}

	return error;
}



// -------------------------------------------------------------------------------------------------
// soporte para DX 9
// -------------------------------------------------------------------------------------------------
CDX9Mesh::CDX9Mesh()
{
	cant_faces = 0;
	start_index = 0;
	strcpy(fname,"");
	device = NULL;
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


bool CDX9Mesh::LoadFromFile(CRenderEngine *p_engine,CDX9Device *p_device,char *filename)
{
	FILE *fp = fopen(filename,"rb");
	if(fp==NULL)
		return false;

	int header[9];
	// leo el header
	fread(&header,sizeof(header),1,fp);
	int version = header[6];

	// Cantidad de layers
	unsigned long g_dwNumMaterials;
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

		// Cargo la textura en el pool (o obtengo el nro de textura si es que ya estaba)
		layers[i].nro_textura = p_engine->LoadTexture(texture_name);
	}


	// Cantidad de caras
	fread(&cant_faces,sizeof(cant_faces),1,fp);
	// Cantidad de vertices
	fread(&cant_vertices,sizeof(cant_vertices),1,fp);
	// Bytes por vertice (usualmente bps = 32 bytes)
	fread(&bpv,sizeof(bpv),1,fp);
	// Vertex buffer
	MESH_VERTEX *pVertices = new MESH_VERTEX[cant_vertices];
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

	int cant_indices = cant_faces*3;
	unsigned long  *pIndices = new unsigned long[cant_indices];
	// Index buffer
	fread(pIndices,cant_indices*sizeof(unsigned long),1,fp);
	// Attributos
	unsigned long  *pAttributes = new unsigned long[cant_faces];
	fread(pAttributes,cant_faces*sizeof(unsigned long),1,fp);
	fclose(fp);


	// create the vertex buffer
	UINT size = sizeof(MESH_VERTEX) * cant_vertices;
	if( FAILED( p_device->g_pd3dDevice->CreateVertexBuffer( size, 0 , 
		0 , D3DPOOL_DEFAULT, &m_vertexBuffer, NULL ) ) )
		return false;

	MESH_VERTEX *p_gpu_vb;
	if( FAILED( m_vertexBuffer->Lock( 0, size, (void**)&p_gpu_vb, 0 ) ) )
		return false;
	memcpy(p_gpu_vb,pVertices,size);
	m_vertexBuffer->Unlock();


	// Index buffer
	size = sizeof(unsigned long) * cant_indices;
	if( FAILED( p_device->g_pd3dDevice->CreateIndexBuffer( size, 0 ,D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_indexBuffer, NULL ) ) )
		return false;

	// El index buffer es mas complicado, porque tengo que dividir por layers. 
	unsigned long  *pIndicesAux = new unsigned long[cant_indices];

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

	// Aprovecho para computar el tamaño y la posicion del mesh
	D3DXVECTOR3 min = D3DXVECTOR3 (10000,10000,10000);
	D3DXVECTOR3 max = D3DXVECTOR3 (-10000,-10000,-10000);
	for(int i=0;i<cant_vertices;++i)
	{
		if(pVertices[i].position.x<min.x)
			min.x = pVertices[i].position.x;
		if(pVertices[i].position.y<min.y)
			min.y = pVertices[i].position.y;
		if(pVertices[i].position.z<min.z)
			min.z = pVertices[i].position.z;

		if(pVertices[i].position.x>max.x)
			max.x = pVertices[i].position.x;
		if(pVertices[i].position.y>max.y)
			max.y = pVertices[i].position.y;
		if(pVertices[i].position.z>max.z)
			max.z = pVertices[i].position.z;
	}

	m_pos = min;
	m_size = max-min;

	// libero la memoria de los vertices, indices y attributos
	delete pVertices;
	delete pIndices;
	delete pIndicesAux;
	delete pAttributes;

	// Actualizo datos internos
	device = p_device;
	engine = p_engine;
	strcpy(fname,filename);


	return true;
}


bool CDX9Mesh::CreateFromXMLData()
{
	CDX9Device *p_device = (CDX9Device *)device;

	// create the vertex buffer
	UINT size = sizeof(MESH_VERTEX) * cant_vertices;
	if( FAILED( p_device->g_pd3dDevice->CreateVertexBuffer( size, 0 , 
		0 , D3DPOOL_DEFAULT, &m_vertexBuffer, NULL ) ) )
		return false;

	// copy the vertices into the buffer
	MESH_VERTEX *pVertices;
	if( FAILED( m_vertexBuffer->Lock( 0, size, (void**)&pVertices, 0 ) ) )
		return false;
	// Aprovecho para computar el tamaño y la posicion del mesh
	D3DXVECTOR3 min = D3DXVECTOR3 (10000,10000,10000);
	D3DXVECTOR3 max = D3DXVECTOR3 (-10000,-10000,-10000);
	for(int i=0;i<cant_vertices;++i)
	{
		int index = coordinatesIdx[i] * 3;
		pVertices[i].position.x = vertices[index];
		pVertices[i].position.y = vertices[index + 1];
		pVertices[i].position.z = vertices[index + 2];

		pVertices[i].normal.x = normals[index];
		pVertices[i].normal.y = normals[index + 1];
		pVertices[i].normal.z = normals[index + 2];

		index = textCoordsIdx[i] * 2;
		pVertices[i].texcoord.x = texCoords[index];
		pVertices[i].texcoord.y = texCoords[index + 1];

		if(pVertices[i].position.x<min.x)
			min.x = pVertices[i].position.x;
		if(pVertices[i].position.y<min.y)
			min.y = pVertices[i].position.y;
		if(pVertices[i].position.z<min.z)
			min.z = pVertices[i].position.z;

		if(pVertices[i].position.x>max.x)
			max.x = pVertices[i].position.x;
		if(pVertices[i].position.y>max.y)
			max.y = pVertices[i].position.y;
		if(pVertices[i].position.z>max.z)
			max.z = pVertices[i].position.z;

	}
	m_vertexBuffer->Unlock();

	m_pos = min;
	m_size = max-min;

	// Index buffer
	int cant_indices = cant_vertices;
	size = sizeof(unsigned long) * cant_indices;
	if( FAILED( p_device->g_pd3dDevice->CreateIndexBuffer( size, 0 ,D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_indexBuffer, NULL ) ) )
		return false;

	unsigned long *pIndicesAux;
	if( FAILED( m_indexBuffer->Lock( 0, size, (void**)&pIndicesAux, 0 ) ) )
		return false;
	for(int i=0;i<cant_indices;++i)
		pIndicesAux[i] = i;
	m_indexBuffer->Unlock();


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



