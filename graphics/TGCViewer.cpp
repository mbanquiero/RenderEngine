#include "stdafx.h"
#include "/dev/graphics/TGCViewer.h"
#include "/dev/graphics/xstring.h"

#define BUFFER_SIZE  600000
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
#define SAFE_DELETE(p) { if ( (p) ) { delete (p); (p) = 0; } }


// Carga un mesh desde el formato xml del TGC viewer. 
// Si el xml tiene una escena completa carga solo el primer mesh, o bien el mesh con el nombre que le paso como parametro

bool CTGCMeshParser::LoadMesh(CMesh *p_mesh,char *filename,char *mesh_name,int mesh_mat_id)
{
	FILE *fp = fopen(filename,"rt");
	if(fp==NULL)
		return false;

	M = p_mesh;

	// Path de texturas x defecto formato xml
	strcpy(path_texturas,filename);
	char *s = strrchr(path_texturas,'/');
	if(s!=NULL)
		*s = '\0';

	if(mesh_name!=NULL)
		strcpy(mesh_id,mesh_name);
	else
		strcpy(mesh_id,"");

	// Inicializo la Data auxiliar para el xml parser
	matIds = coordinatesIdx = textCoordsIdx = NULL;
	vertices = normals = texCoords = NULL;
	xml_current_tag = xml_current_layer = -1;
	matIdsCount = 0;
	xml_multimaterial = false;
	xml_mat_id = mesh_mat_id;
	xml_current_mat_id = -1;

	// Leo y parseo el xml
	char *buffer = new char[BUFFER_SIZE];
	bool terminar = false;
	while(fgets(buffer,BUFFER_SIZE,fp)!=NULL && !terminar)
	{
		ltrim(buffer);
		if(ParseXMLLine(buffer)==-1)
			terminar = true;
	}
	delete []buffer,
	fclose(fp);

	// Creo los datos internos del mesh
	CreateMeshData();

	// libero los datos auxiliares
	SAFE_DELETE(coordinatesIdx);
	SAFE_DELETE(textCoordsIdx);
	SAFE_DELETE(matIds);
	SAFE_DELETE(vertices);
	SAFE_DELETE(normals);
	SAFE_DELETE(texCoords);

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
char CTGCMeshParser::ParseXMLLine(char *buffer)
{
	char procesada = 0;
	if(strncmp(buffer,"<m name=" , 8)==0)
	{

		// los materiales estan numerados del cero a mat_count - 1
		xml_current_mat_id++;
		if(xml_mat_id==-1 || xml_mat_id==xml_current_mat_id)
		{
			// Determino si tiene sub material o es un unico material
			if(strstr(buffer,"'Multimaterial'")!= NULL)
				// tiene submateriales, 
					xml_multimaterial = true;
			else
			{
				// Agrego el unico layer
				xml_current_tag = 0;
				xml_current_layer = M->cant_layers++;
				memset(&M->layers[xml_current_layer],0,sizeof(MESH_LAYER));
			}
		}
		else
			xml_current_tag = -1;

		procesada = 1;
	}

	if(strncmp(buffer,"<subM name=" , 11)==0)
	{
		if(xml_mat_id==-1 || xml_mat_id==xml_current_mat_id)
		{
			// se supone que xml_multimaterial == true
			// Agrego un layer de material
			xml_current_tag = 0;
			xml_current_layer = M->cant_layers++;
			memset(&M->layers[xml_current_layer],0,sizeof(MESH_LAYER));
		}
		else
			xml_current_tag = -1;
		procesada = 1;
	}

	if(strncmp(buffer,"<ambient>" , 9)==0)
	{
		if(xml_current_tag==0 && xml_current_layer>=0 && xml_current_layer<M->cant_layers)
			M->layers[xml_current_layer].Ambient = ParseXMLColor(buffer + 10);
		procesada = 1;
	}

	if(strncmp(buffer,"<diffuse>" , 9)==0)
	{
		if(xml_current_tag==0 && xml_current_layer>=0 && xml_current_layer<M->cant_layers)
			M->layers[xml_current_layer].Diffuse = ParseXMLColor(buffer + 10);
		procesada = 1;
	}

	if(strncmp(buffer,"<specular>" , 10)==0)
	{
		if(xml_current_tag==0 && xml_current_layer>=0 && xml_current_layer<M->cant_layers)
		{
			D3DCOLORVALUE Specular = ParseXMLColor(buffer + 11);
			M->layers[xml_current_layer].ke = Specular.b;
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<opacity>" , 9)==0)
	{
		if(xml_current_tag==0 && xml_current_layer>=0 && xml_current_layer<M->cant_layers)
			M->layers[xml_current_layer].kt = atof(buffer + 10);
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
				if(xml_current_tag==0 && xml_current_layer>=0 && xml_current_layer<M->cant_layers)
				{
					// Cargo la textura en el pool (o obtengo el nro de textura si es que ya estaba)
					//layers[xml_current_layer].nro_textura = engine->LoadTexture(texture_name);
					strcpy(M->layers[xml_current_layer].texture_name , texture_name);
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
					{
						// econtre el mesh que quiere cargar, pongo el status en cargar mesh (=1)
						xml_current_tag = 1;

					}
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
				M->cant_vertices = count;
				coordinatesIdx = new int[count];
				char *p = strchr(buffer+23,'>');
				if(p!=NULL)
					ParseIntStream(p+1,coordinatesIdx,count);
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
					ParseIntStream(p+1,textCoordsIdx,count);
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
					ParseFloatStream(p+1,vertices,count);
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
					ParseFloatStream(p+1,normals,count);
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
					ParseFloatStream(p+1,texCoords,count);
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
				{
					ParseIntStream(p+1,matIds,count);
					matIdsCount = count;
				}
			}
		}
		procesada = 1;
	}

	// Le doy la oportunidad al caller de seguir procesando la linea
	return procesada;
}


// Crea los datos internos del mesh desde los datos que leyo del xml
void CTGCMeshParser::CreateMeshData()
{
	// Cargo la estructura de vertices
	int cant_vertices = M->cant_vertices;
	MESH_VERTEX *pVertices = M->pVertices = new MESH_VERTEX[cant_vertices];
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
	}

	// Cargo la estructura de indices (	el index buffer es trivial)
	M->cant_indices = cant_vertices;
	DWORD *pIndices = M->pIndices = new DWORD[M->cant_indices];
	for(int i=0;i<M->cant_indices;++i)
		pIndices[i] = i;

	// el buffer de atributos hay 2 casos
	M->cant_faces = cant_vertices/3;
	DWORD *pAttributes = M->pAttributes = new DWORD[M->cant_faces];
	if(M->cant_layers==1 || matIdsCount<=1)
	{
		// y el todos tienen el mismo layer, lo lleno con ceros
		memset(pAttributes,0,sizeof(DWORD)*M->cant_faces);
	}
	else
	{
		// tiene multimateriales, lo cargo desde los matids
		for(int i=0;i<M->cant_faces;++i)
			pAttributes[i] = matIds[i];
	}

	/*
	// Cargo la estructura de indices
	int cant_indices = cant_vertices;
	DWORD *pIndices = M->pIndices = new DWORD[cant_indices];
	if(M->cant_layers==1 || matIdsCount<=1)
	{
		// index buffer trivial
		for(int i=0;i<cant_indices;++i)
			pIndicesAux[i] = i;
		
		if(xml_mat_id!=-1 && M->cant_layers>1)
			// el mesh cargo varios layers, pero de verdad tiene uno solo, es que el dice matid.
			// lo que hago es dejar el index buffer trivial, y copiar el layer id, en el primer layer,
			// los demas layers no sirven
			M->layers[0] = M->layers[xml_mat_id];

		M->layers[0].start_index = 0;
		M->layers[0].cant_indices = cant_indices;
		M->cant_layers = 1;
	}
	else
	{
		// Organizo por layer
		int t = 0;
		for(int n = 0 ; n< M->cant_layers ;++n)
		{
			// inicializo el layer
			M->layers[n].start_index = t;
			M->layers[n].cant_indices = 0;
			// Cada 3 indices hay un attributo que indica el nro de layer, un attrib x face
			int cant_faces = cant_indices / 3;
			for(int i=0;i<cant_faces;++i)
			{
				int nro_layer = matIds[i];
				if(nro_layer==n)
				{
					pIndicesAux[t++] = i*3;
					pIndicesAux[t++] = i*3+1;
					pIndicesAux[t++] = i*3+2;
					M->layers[n].cant_indices += 3;
				}
			}
		}
	}
	*/
}



// helpers para xml
// [150.0,150.0,150.0,255.0]
D3DCOLORVALUE CTGCXmlParser::ParseXMLColor(char *buffer)
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
D3DXVECTOR3 CTGCXmlParser::ParseXMLVector3(char *buffer)
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

D3DXVECTOR4 CTGCXmlParser::ParseXMLVector4(char *buffer)
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

bool CTGCXmlParser::ParseIntStream(char *buffer,int *S,int count)
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


bool CTGCXmlParser::ParseFloatStream(char *buffer,float *S,int count)
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



int CTGCMeshParser::LoadSceneHeader(char *filename,tgc_scene_mesh mesh_lst[])
{
	FILE *fp = fopen(filename,"rt");
	if(fp==NULL)
		return 0;

	// Leo y parseo el xml
	int cant = 0;

	char *buffer = new char[BUFFER_SIZE];
	while(fgets(buffer,BUFFER_SIZE,fp)!=NULL)
	{
		ltrim(buffer);
		if(strncmp(buffer,"<mesh name=" , 11)==0)
		{

			// busco la posicion 
			char *p = strstr(buffer,"pos=");
			if(p!=NULL)
			{
				D3DXVECTOR3 pos = ParseXMLVector3(p+6);
				mesh_lst[cant].pos.x= pos.x;
				mesh_lst[cant].pos.y = pos.z;
				mesh_lst[cant].pos.z = pos.y;
				
			}

			// tomo el nombre del mesh
			p = strchr(buffer+12,'\'');
			if(p!=NULL)
			{
				*p = 0;
				strcpy(mesh_lst[cant].mesh_id,buffer+12);

				// y que material tiene que cargar
				char *q = strstr(p+1,"matId=");
				if(q!=NULL)
					mesh_lst[cant].mat_id = atoi(q + 7);
				else
					mesh_lst[cant].mat_id = -1;
				++cant;
			}
		}
	}
	delete []buffer,
	fclose(fp);

	return cant;
}


bool CTGCSkeletalMeshParser::LoadSkeletalMesh(CSkeletalMesh *p_mesh,char *filename)
{
	FILE *fp = fopen(filename,"rt");
	if(fp==NULL)
		return false;

	M = p_mesh;

	// Path de texturas x defecto formato xml
	strcpy(path_texturas,filename);
	char *s = strrchr(path_texturas,'/');
	if(s!=NULL)
		*s = '\0';

	// Inicializo la Data auxiliar para el xml parser
	coordinatesIdx = textCoordsIdx = NULL;
	vertices = normals = tangents = binormals = texCoords = NULL;
	xml_current_tag = xml_current_layer = -1;
	xml_mat_id = -1;
	strcpy(mesh_id,"");

	// Leo y parseo el xml
	char *buffer = new char[BUFFER_SIZE];
	bool terminar = false;
	while(fgets(buffer,BUFFER_SIZE,fp)!=NULL && !terminar)
	{
		ltrim(buffer);
		if(ParseXMLLine(buffer)==-1)
			terminar = true;
	}
	delete []buffer,
	fclose(fp);

	// Creo los datos internos del mesh
	CreateMeshData();

	// libero los datos auxiliares
	SAFE_DELETE(coordinatesIdx);
	SAFE_DELETE(textCoordsIdx);
	SAFE_DELETE(vertices);
	SAFE_DELETE(normals);
	SAFE_DELETE(binormals);
	SAFE_DELETE(aux_verticesWeights);
	SAFE_DELETE(tangents);
	SAFE_DELETE(texCoords);


	return true;
}


// Es similar al xml del mesh comun pero agrega la estructura de bones que se explica sola, y los pesos, que es un poco mas tricky
// <weights count='7974'>0 4 1.0 
// vienen en pares de 3, el indice del vertice, el indice del hueso, y el peso p dicho.
// Ojo que el indice del vertice es del coordenadas del vertice, no el del vertice pp dicho que hay que ir armandolo
// Por eso usa una estructura auxiliar, que permite acceder rapidamente a los pesos de un vertice en el modelo xml
// para pasarlo a los pesos del vertice en el modelo propio. 
char CTGCSkeletalMeshParser::ParseXMLLine(char *buffer)
{
	char rta = CTGCMeshParser::ParseXMLLine(buffer);
	if(rta)
		return rta;		// listo		(o bien ya proceso la linea o bien devuelve -1 indicando que termina
	
	char procesada = 0;

	CSkeletalMesh *p_mesh = (CSkeletalMesh *)M;
	
	if(strncmp(buffer,"<binormals count=" , 17)==0)
	{
		int count = atoi(buffer + 18);
		if(count>0)
		{
			binormals = new FLOAT[count];
			char *p = strchr(buffer+18,'>');
			if(p!=NULL)
				CTGCXmlParser::ParseFloatStream(p+1,binormals,count);
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<tangents count=" , 16)==0)
	{
		int count = atoi(buffer + 17);
		if(count>0)
		{
			tangents = new FLOAT[count];
			char *p = strchr(buffer+17,'>');
			if(p!=NULL)
				CTGCXmlParser::ParseFloatStream(p+1,tangents,count);
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<skeleton bonesCount=" , 21)==0)
	{
		p_mesh->cant_bones= atoi(buffer + 22);
		procesada = 1;
	}

	if(strncmp(buffer,"<bone id=" , 9)==0)
	{
		int bone_id = atoi(buffer + 10);
		if(bone_id>=0 && bone_id<p_mesh->cant_bones)
		{
			p_mesh->bones[bone_id].id = bone_id;
			// Busco el nombre 
			char *p = strstr(buffer , "name='");
			if(p!=NULL)
			{
				char *q = strchr(p+6,'\'');
				if(q!=NULL)
				{
					int len = q-p - 6;
					strncpy(p_mesh->bones[bone_id].name, p+6,len);
					p_mesh->bones[bone_id].name[len] = '\0';
				}
			}					

			// Busco el PADRE
			p = strstr(buffer , "parentId='");
			if(p!=NULL)
			{
				p_mesh->bones[bone_id].parentId = atoi(p + 10);
			}					

			// Posicion
			p = strstr(buffer , "pos='");
			if(p!=NULL)
			{
				p_mesh->bones[bone_id].startPosition = CTGCXmlParser::ParseXMLVector3(p+6);
			}					

			// Orientacion
			p = strstr(buffer , "rotQuat='");
			if(p!=NULL)
			{
				D3DXVECTOR4 rot = CTGCXmlParser::ParseXMLVector4(p+10);
				p_mesh->bones[bone_id].startRotation.x = rot.x;
				p_mesh->bones[bone_id].startRotation.y = rot.y;
				p_mesh->bones[bone_id].startRotation.z = rot.z;
				p_mesh->bones[bone_id].startRotation.w = rot.w;
			}					

			// Computo la matriz local en base a la orientacion del cuaternion y la traslacion
			p_mesh->bones[bone_id].computeMatLocal();
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<weights count=" , 15)==0)
	{
		int count = atoi(buffer + 16);

		char *p = strchr(buffer+16,'>');
		if(p!=NULL)
		{
			float *valores = new float[count];
			int cant_weights =  count/ 3;
			aux_verticesWeights = new vertexWeight[p_mesh->cant_vertices];
			memset(aux_verticesWeights,0,sizeof(vertexWeight)*p_mesh->cant_vertices);
			int *wxv = new int[p_mesh->cant_vertices];			// Auxiliar weiths x vertex (maximo 4 weights x vertice)
			memset(wxv,0,sizeof(int)*p_mesh->cant_vertices);

			CTGCXmlParser::ParseFloatStream(p+1,valores,count);
			for(int i=0;i<cant_weights;++i)
			{
				int vertex_index = valores[i*3];
				int j = wxv[vertex_index]++;
				if(j>=0 && j<4)
				{
					aux_verticesWeights[vertex_index].boneIndex[j]  = valores[i*3+1];
					aux_verticesWeights[vertex_index].weight[j] = valores[i*3+2];
				}
			}
			delete []valores;
			delete []wxv;
		}
		procesada = 1;
	}
	return procesada;
}



void CTGCSkeletalMeshParser::CreateMeshData()
{
	CSkeletalMesh *p_mesh = (CSkeletalMesh *)M;

	// Cargo la estructura de vertices
	int cant_vertices = p_mesh->cant_vertices;
	SKELETAL_MESH_VERTEX *pVertices = p_mesh->pVertices = new SKELETAL_MESH_VERTEX[cant_vertices];
	p_mesh->verticesWeights = new vertexWeight[p_mesh->cant_vertices];
	for(int i=0;i<p_mesh->cant_vertices;++i)
	{
		int index = coordinatesIdx[i] * 3;
		pVertices[i].position.x = vertices[index];
		pVertices[i].position.y = vertices[index + 1];
		pVertices[i].position.z = vertices[index + 2];

		index = i*3;
		pVertices[i].normal.x = normals[index];
		pVertices[i].normal.y = normals[index + 1];
		pVertices[i].normal.z = normals[index + 2];

		pVertices[i].binormal.x = binormals[index];
		pVertices[i].binormal.y = binormals[index + 1];
		pVertices[i].binormal.z = binormals[index + 2];

		pVertices[i].tangent.x = tangents[index];
		pVertices[i].tangent.y = tangents[index + 1];
		pVertices[i].tangent.z = tangents[index + 2];

		index = coordinatesIdx[i];
		pVertices[i].blendIndices.x = aux_verticesWeights[index].boneIndex[0];
		pVertices[i].blendIndices.y = aux_verticesWeights[index].boneIndex[1];
		pVertices[i].blendIndices.z = aux_verticesWeights[index].boneIndex[2];
		pVertices[i].blendIndices.w = aux_verticesWeights[index].boneIndex[3];

		pVertices[i].blendWeights.x = aux_verticesWeights[index].weight[0];
		pVertices[i].blendWeights.y = aux_verticesWeights[index].weight[1];
		pVertices[i].blendWeights.z = aux_verticesWeights[index].weight[2];
		pVertices[i].blendWeights.w = aux_verticesWeights[index].weight[3];

		p_mesh->verticesWeights[i] = aux_verticesWeights[index];

		index = textCoordsIdx[i] * 2;
		pVertices[i].texcoord.x = texCoords[index];
		pVertices[i].texcoord.y = texCoords[index + 1];
	}


	// Cargo la estructura de indices (	el index buffer es trivial)
	M->cant_indices = cant_vertices;
	DWORD *pIndices = M->pIndices = new DWORD[M->cant_indices];
	for(int i=0;i<M->cant_indices;++i)
		pIndices[i] = i;

	// el buffer de atributos hay 2 casos
	M->cant_faces = cant_vertices/3;
	DWORD *pAttributes = M->pAttributes = new DWORD[M->cant_faces];
	if(M->cant_layers==1 || matIdsCount<=1)
	{
		// y el todos tienen el mismo layer, lo lleno con ceros
		memset(pAttributes,0,sizeof(DWORD)*M->cant_faces);
	}
	else
	{
		// tiene multimateriales, lo cargo desde los matids
		for(int i=0;i<M->cant_faces;++i)
			pAttributes[i] = matIds[i];
	}
}

