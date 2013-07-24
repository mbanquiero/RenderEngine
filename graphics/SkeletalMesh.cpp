#include "stdafx.h"
#include "/dev/graphics/mesh.h"
#include "/dev/graphics/SkeletalMesh.h"
#include "/dev/graphics/device.h"
#include "/dev/graphics/dx11device.h"
#include "/dev/graphics/dx9device.h"
#include "/dev/graphics/RenderEngine.h"

#define BUFFER_SIZE  600000
D3DCOLORVALUE ParserXMLColor(char *buffer);
bool ParserIntStream(char *buffer,int *S,int count);
bool ParserFloatStream(char *buffer,float *S,int count);
D3DXVECTOR3 ParserXMLVector3(char *buffer);
D3DXVECTOR4 ParserXMLVector4(char *buffer);

CSkeletalBone::CSkeletalBone()
{
	Clean();
}

void CSkeletalBone::Clean()
{
	parentId = id = -1;
	strcpy(name,"");
	startPosition = D3DXVECTOR3(0,0,0);
	startRotation = D3DXQUATERNION(0,0,0,0);
	D3DXMatrixIdentity(&matLocal);
	matFinal = matInversePose = matLocal;
}


// Calculo la matriz local
void CSkeletalBone::computeMatLocal()
{
	D3DXMATRIX R;
	D3DXMatrixRotationQuaternion(&R,&startRotation);
	D3DXMATRIX T;
	D3DXMatrixTranslation(&T,startPosition.x,startPosition.y,startPosition.z);
	matLocal = R*T;
}


CBaseSkeletalMesh::CBaseSkeletalMesh() : CBaseMesh()
{
	verticesWeights = NULL;
	cant_animaciones = 0;

}

CBaseSkeletalMesh::~CBaseSkeletalMesh()
{
	SAFE_DELETE(verticesWeights);
	for(int i=0;i<cant_animaciones;++i)
		SAFE_DELETE(animacion[i]);
}




bool CBaseSkeletalMesh::LoadFromXMLFile(CRenderEngine *p_engine,CDevice *p_device,char *filename)
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

	// Inicializo la Data auxiliar para el xml parser
	coordinatesIdx = textCoordsIdx = NULL;
	vertices = normals = tangents = binormals = texCoords = NULL;
	xml_current_tag = xml_current_layer = -1;


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
	SAFE_DELETE(vertices);
	SAFE_DELETE(normals);
	SAFE_DELETE(binormals);
	SAFE_DELETE(aux_verticesWeights);
	SAFE_DELETE(tangents);
	SAFE_DELETE(texCoords);

	// Actualizo datos internos
	cant_faces = cant_vertices / 3;

	// Setup inicial del esqueleto
	setupSkeleton();

	// Busco en el directorio todas las animaciones que tiene
	char path_animaciones[MAX_PATH];
	strcpy(path_animaciones,filename);
	char *p = strrchr(path_animaciones,'/');
	if(p!=NULL)
	{
		*p = 0;
		char cur_dir[MAX_PATH];
		GetCurrentDirectory(sizeof(cur_dir),cur_dir);

		// 1ero en el directorio donde esta
		SetCurrentDirectory(path_animaciones);
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind = FindFirstFile("*-TgcSkeletalAnim.xml",&FindFileData);
		if (hFind != INVALID_HANDLE_VALUE) 
		{
			while(FindNextFile(hFind, &FindFileData) != 0) 
				LoadAnimation(FindFileData.cFileName);
			FindClose(hFind);
		}

		// por ultimo en la carpeta Animations
		strcat(path_animaciones,"/Animations");
		SetCurrentDirectory(path_animaciones);
		hFind = FindFirstFile("*-TgcSkeletalAnim.xml",&FindFileData);
		if (hFind != INVALID_HANDLE_VALUE) 
		{
			while(FindNextFile(hFind, &FindFileData) != 0) 
				LoadAnimation(FindFileData.cFileName);
			FindClose(hFind);
		}
		SetCurrentDirectory(cur_dir);
	}

	return true;
}

// Es similar al xml del mesh comun pero agrega la estructura de bones que se explica sola, y los pesos, que es un poco mas tricky
// <weights count='7974'>0 4 1.0 
// vienen en pares de 3, el indice del vertice, el indice del hueso, y el peso p dicho.
// Ojo que el indice del vertice es del coordenadas del vertice, no el del vertice pp dicho que hay que ir armandolo
// Por eso usa una estructura auxiliar, que permite acceder rapidamente a los pesos de un vertice en el modelo xml
// para pasarlo a los pesos del vertice en el modelo propio. 

char CBaseSkeletalMesh::ParserXMLLine(char *buffer)
{
	char rta = CBaseMesh::ParserXMLLine(buffer);
	if(rta)
		return rta;		// listo		(o bien ya proceso la linea o bien devuelve -1 indicando que termina
	
	char procesada = 0;
	
	if(strncmp(buffer,"<binormals count=" , 17)==0)
	{
		int count = atoi(buffer + 18);
		if(count>0)
		{
			binormals = new FLOAT[count];
			char *p = strchr(buffer+18,'>');
			if(p!=NULL)
				ParserFloatStream(p+1,binormals,count);
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
				ParserFloatStream(p+1,tangents,count);
		}
		procesada = 1;
	}

	if(strncmp(buffer,"<skeleton bonesCount=" , 21)==0)
	{
		cant_bones= atoi(buffer + 22);
		procesada = 1;
	}

	if(strncmp(buffer,"<bone id=" , 9)==0)
	{
		int bone_id = atoi(buffer + 10);
		if(bone_id>=0 && bone_id<cant_bones)
		{
			bones[bone_id].id = bone_id;
			// Busco el nombre 
			char *p = strstr(buffer , "name='");
			if(p!=NULL)
			{
				char *q = strchr(p+6,'\'');
				if(q!=NULL)
				{
					int len = q-p - 6;
					strncpy(bones[bone_id].name, p+6,len);
					bones[bone_id].name[len] = '\0';
				}
			}					

			// Busco el PADRE
			p = strstr(buffer , "parentId='");
			if(p!=NULL)
			{
				bones[bone_id].parentId = atoi(p + 10);
			}					

			// Posicion
			p = strstr(buffer , "pos='");
			if(p!=NULL)
			{
				bones[bone_id].startPosition = ParserXMLVector3(p+6);
			}					

			// Orientacion
			p = strstr(buffer , "rotQuat='");
			if(p!=NULL)
			{
				D3DXVECTOR4 rot = ParserXMLVector4(p+10);
				bones[bone_id].startRotation.x = rot.x;
				bones[bone_id].startRotation.y = rot.y;
				bones[bone_id].startRotation.z = rot.z;
				bones[bone_id].startRotation.w = rot.w;
			}					

			// Computo la matriz local en base a la orientacion del cuaternion y la traslacion
			bones[bone_id].computeMatLocal();
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
			aux_verticesWeights = new vertexWeight[cant_vertices];
			memset(aux_verticesWeights,0,sizeof(vertexWeight)*cant_vertices);
			int *wxv = new int[cant_vertices];			// Auxiliar weiths x vertex (maximo 4 weights x vertice)
			memset(wxv,0,sizeof(int)*cant_vertices);

			ParserFloatStream(p+1,valores,count);
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
			delete valores;
			delete wxv;
		}
		procesada = 1;
	}

	return procesada;
}


bool CBaseSkeletalMesh::LoadAnimation(char *fname)
{
	bool rta = false;
	CSkeletalAnimation *p_animacion = new CSkeletalAnimation();
	if(p_animacion->CreateFromFile(fname))
	{
		rta = true;
		animacion[cant_animaciones++] = p_animacion;
	}
	return rta;
}


CSkeletalAnimation::CSkeletalAnimation()
{
	strcpy(name,"");
	cant_bones = 0;
	cant_frames = 0;
	frame_rate = 0;
	memset(bone_animation, 0,sizeof(st_bone_animation)*MAX_BONES);
}


bool CSkeletalAnimation::CreateFromFile(char *fname)
{
	FILE *fp = fopen(fname,"rt");
	if(fp==NULL)
		return false;

	// Leo y parseo el xml
	bone_id = 0;
	nro_frame = 0;
	char buffer[1024];
	while(fgets(buffer,sizeof(buffer),fp)!=NULL)
	{
		ltrim(buffer);
		ParserXMLLine(buffer);
	}
	fclose(fp);
	return true;
}


bool CSkeletalAnimation::ParserXMLLine(char *buffer)
{
	bool procesada = true;
	if(strncmp(buffer,"<animation name=" , 16)==0)
	{
		char *p = strchr(buffer+18 , '\'');
		if(p!=NULL)
		{
			int len = p - buffer - 16;
			strncpy(name,buffer+17,len);
			name[len] = '\0';
		}

		p = strstr(buffer , " framesCount='");
		if(p!=NULL)
			cant_frames = atoi(p+14);

		p = strstr(buffer , " frameRate=");
		if(p!=NULL)
			frame_rate = atoi(p+12);


	}						
	if(strncmp(buffer,"<bone id=" , 9)==0)
	{
		bone_id = atoi(buffer + 10);

	}
	if(strncmp(buffer,"<frame n=" , 9)==0)
	{
		nro_frame = atoi(buffer + 10);
		int n = bone_animation[bone_id].cant_frames++;
		bone_animation[bone_id].frame[n].nro_frame = nro_frame;

		// Posicion
		char *p = strstr(buffer , "pos='");
		if(p!=NULL)
		{
			bone_animation[bone_id].frame[n].Position = ParserXMLVector3(p+6);;
		}					

		// Orientacion
		p = strstr(buffer , "rotQuat='");
		if(p!=NULL)
		{
			D3DXVECTOR4 rot = ParserXMLVector4(p+10);
			bone_animation[bone_id].frame[n].Rotation.x = rot.x;
			bone_animation[bone_id].frame[n].Rotation.y = rot.y;
			bone_animation[bone_id].frame[n].Rotation.z = rot.z;
			bone_animation[bone_id].frame[n].Rotation.w = rot.w;
		}					
	}

	return procesada;
}




void CBaseSkeletalMesh::setupSkeleton()
{
	//Actualizar jerarquia
	float det;
	for (int i = 0; i < cant_bones; i++)
	{
		int parent_id = bones[i].parentId;
		if(parent_id==-1)
			bones[i].matFinal = bones[i].matLocal;
		else
			bones[i].matFinal = bones[i].matLocal * bones[parent_id].matFinal;
		//Almacenar la inversa de la posicion original del hueso, para la referencia inicial de los vertices
		D3DXMatrixInverse(&bones[i].matInversePose , &det, &bones[i].matFinal);
	}
}

void CBaseSkeletalMesh::initAnimation(int nro_animacion, bool con_loop, float userFrameRate)
{
	animating = true;
	currentAnimation = nro_animacion;
	CSkeletalAnimation *p_animacion = animacion[nro_animacion];
	playLoop = playLoop;
	currentTime = 0;
	currentFrame = 0;
	frameRate = userFrameRate > 0 ? userFrameRate : p_animacion->frame_rate;
	animationTimeLenght = ((float)p_animacion->cant_frames - 1) / frameRate;

	//Configurar posicion inicial de los huesos
	for (int i = 0; i < cant_bones; i++)
	{
		//Determinar matriz local inicial
		st_bone_frame frame0 = p_animacion->bone_animation[i].frame[0];
		D3DXMATRIX R;
		D3DXMatrixRotationQuaternion(&R,&frame0.Rotation);
		D3DXMATRIX T;
		D3DXMatrixTranslation(&T,frame0.Position.x,frame0.Position.y,frame0.Position.z);
		bones[i].matLocal = R*T;
		//Multiplicar por matriz del padre, si tiene
		int parent_id = bones[i].parentId;
		if(parent_id != -1)
			bones[i].matFinal = bones[i].matLocal * bones[parent_id].matFinal;
		else
			bones[i].matFinal = bones[i].matLocal;
	}

	//Ajustar vertices a posicion inicial del esqueleto
	updateMeshVertices();
}




/// Actualizar los vertices de la malla segun las posiciones del los huesos del esqueleto
void CBaseSkeletalMesh::updateMeshVertices()
{
	//Precalcular la multiplicación para llevar a un vertice a Bone-Space y luego transformarlo segun el hueso
	//Estas matrices se envian luego al Vertex Shader para hacer skinning en GPU
	for (int i = 0; i < cant_bones; i++)
		matBoneSpace[i] = bones[i].matInversePose * bones[i].matFinal;
}



/// Actualiza el cuadro actual de la animacion.
void CBaseSkeletalMesh::updateAnimation()
{
	//Ver que haya transcurrido cierta cantidad de tiempo
	if(engine->elpased_time < 0)
		return;

	//Sumo el tiempo transcurrido
	currentTime += engine->elpased_time;

	//Se termino la animacion
	if (currentTime > animationTimeLenght)
	{
		//Ver si hacer loop
		if (playLoop)
		{
			//Dejar el remanente de tiempo transcurrido para el proximo loop
			currentTime = fmod(currentTime , animationTimeLenght);
			//setSkleletonLastPose();
			//updateMeshVertices();
		}
		else
		{

			//TODO: Puede ser que haya que quitar este stopAnimation() y solo llamar al Listener (sin cargar isAnimating = false)
			//stopAnimation();
		}
	}

	//La animacion continua
	else
	{
		//Actualizar esqueleto y malla
		updateSkeleton();
		updateMeshVertices();
	}
}



/// Actualiza la posicion de cada hueso del esqueleto segun sus KeyFrames de la animacion
void CBaseSkeletalMesh::updateSkeleton()
{

	CSkeletalAnimation *p_animacion = animacion[currentAnimation];
	for (int i = 0; i < cant_bones; i++)
	{
		//Tomar el frame actual para este hueso
		st_bone_animation boneFrames = p_animacion->bone_animation[i];
		if(boneFrames.cant_frames == 1)
			continue;		//Solo hay un frame, no hacer nada, ya se hizo en el init de la animacion

		//Obtener cuadro actual segun el tiempo transcurrido
		float currentFrameF = currentTime * frameRate;
		//Ve a que KeyFrame le corresponde
		int keyFrameIdx = getCurrentFrameBone(&boneFrames, currentFrameF);
		currentFrame = keyFrameIdx;

		//Armar un intervalo entre el proximo KeyFrame y el anterior
		st_bone_frame *p_frame1 = &boneFrames.frame[keyFrameIdx - 1];
		st_bone_frame *p_frame2 = &boneFrames.frame[keyFrameIdx];

		//Calcular la cantidad que hay interpolar en base al la diferencia entre cuadros
		float framesDiff = p_frame2->nro_frame - p_frame1->nro_frame;
		float interpolationValue = (currentFrameF - p_frame1->nro_frame) / framesDiff;

		//Interpolar traslacion
		D3DXVECTOR3  frameTranslation = (p_frame2->Position - p_frame1->Position) * interpolationValue + p_frame1->Position;

		//Interpolar rotacion con SLERP
		D3DXQUATERNION quatFrameRotation;
		D3DXQuaternionSlerp(&quatFrameRotation,&p_frame1->Rotation,&p_frame2->Rotation,interpolationValue);

		//Unir ambas transformaciones de este frame
		D3DXMATRIX R;
		D3DXMatrixRotationQuaternion(&R,&quatFrameRotation);
		D3DXMATRIX T;
		D3DXMatrixTranslation(&T,frameTranslation.x,frameTranslation.y,frameTranslation.z);
		D3DXMATRIX frameMatrix = R*T;

		//Multiplicar por la matriz del padre, si tiene
		int parent_id = bones[i].parentId;
		if(parent_id!=-1)
			bones[i].matFinal = frameMatrix * bones[parent_id].matFinal;
		else
			bones[i].matFinal= frameMatrix;
	}
}


/// Obtener el KeyFrame correspondiente a cada hueso segun el tiempo transcurrido
int CBaseSkeletalMesh::getCurrentFrameBone(st_bone_animation *boneFrames, float currentFrame)
{
	for (int i = 0; i < boneFrames->cant_frames; i++)
	{
		if (currentFrame < boneFrames->frame[i].nro_frame)
		{
			return i;
		}
	}
	return boneFrames->cant_frames - 1;
}


//---------------------------------------------------------------------------------

CDX11SkeletalMesh::CDX11SkeletalMesh() : CBaseSkeletalMesh()
{
	m_vertexBuffer = m_indexBuffer = NULL;
}

CDX11SkeletalMesh::~CDX11SkeletalMesh()
{
	Release();
}

void CDX11SkeletalMesh::Release()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
}


void CDX11SkeletalMesh::SetVertexDeclaration()
{
	CDX11Device *p_device = (CDX11Device *) device;
	p_device->devcon->IASetInputLayout(p_device->pLayoutSkeletalMesh);
	bpv = sizeof(SKELETAL_MESH_VERTEX);
}

// set the shader objects
void CDX11SkeletalMesh::SetShaders()
{
	CDX11Device *p_device = (CDX11Device *) device;
	ID3D11DeviceContext *devcon = p_device->devcon;

	devcon->VSSetShader(p_device->pSkeletalMeshVS, 0, 0);
	devcon->PSSetShader(p_device->pSkeletalMeshPS, 0, 0);

	// Tengo que enviar las matrices al cbuffer del shader
	/*
	matBoneSpace[3]._11 = 0;
	matBoneSpace[3]._12 = 1;
	matBoneSpace[3]._13 = 0;
	*/
	D3DXMATRIX bonesMatWorldArray[MAX_BONES];
	for(int i=0;i<MAX_BONES;++i)
	{
		//D3DXMatrixIdentity(&bonesMatWorldArray[i]);
		//D3DXMatrixTranspose(&bonesMatWorldArray[i],&bonesMatWorldArray[i]);
		D3DXMatrixTranspose(&bonesMatWorldArray[i],&matBoneSpace[i]);
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	boneBufferType * dataPtr;
	devcon->Map(p_device->m_boneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (boneBufferType*)mappedResource.pData;
	for(int i=0;i<MAX_BONES;++i)
		dataPtr->bonesMatWorldArray[i] = bonesMatWorldArray[i];
	devcon->Unmap(p_device->m_boneBuffer, 0);
	// Finanly set the constant buffer in the vertex shader with the updated values.
	devcon->VSSetConstantBuffers(2, 1, &p_device->m_boneBuffer);
	devcon->PSSetConstantBuffers(2, 1, &p_device->m_boneBuffer);

}


void CDX11SkeletalMesh::Draw()
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

void CDX11SkeletalMesh::DrawSubset(int i)
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


bool CDX11SkeletalMesh::CreateFromXMLData()
{
	CDX11Device *p_device = (CDX11Device *)device;
	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(SKELETAL_MESH_VERTEX) * cant_vertices;             
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

	verticesWeights = new vertexWeight[cant_vertices];
	SKELETAL_MESH_VERTEX *pVertices = (SKELETAL_MESH_VERTEX*)ms.pData;
	for(int i=0;i<cant_vertices;++i)
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

		verticesWeights[i] = aux_verticesWeights[index];

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

	if(cant_layers==1)
	{
		// index buffer trivial
		for(int i=0;i<cant_indices;++i)
			pIndicesAux[i] = i;

		layers[0].start_index = 0;
		layers[0].cant_indices = cant_vertices;			// cant_indices == cant_vertices 
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



//---------------------------------------------------------------------------------
CDX9SkeletalMesh::CDX9SkeletalMesh() : CBaseSkeletalMesh()
{
	m_vertexBuffer = NULL;
	m_indexBuffer = NULL;
}

CDX9SkeletalMesh::~CDX9SkeletalMesh()
{
	Release();
}

void CDX9SkeletalMesh::Release()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
}


void CDX9SkeletalMesh::SetVertexDeclaration()
{
	CDX9Device *p_device = (CDX9Device *)device;
	LPDIRECT3DDEVICE9 g_pd3dDevice = p_device->g_pd3dDevice;
	g_pd3dDevice->SetVertexDeclaration(p_device->m_pSkeletalMeshVertexDeclaration);
	bpv = sizeof(SKELETAL_MESH_VERTEX);
}

// set the shader objects
void CDX9SkeletalMesh::SetShaders()
{

	CDX9Device *p_device = (CDX9Device *)device;
	LPDIRECT3DDEVICE9 g_pd3dDevice = p_device->g_pd3dDevice;
	p_device->g_pEffect = p_device->g_pEffectStandard;
	p_device->g_pEffect->SetTechnique("SkeletalRender");
	p_device->g_pEffect->SetMatrixArray("bonesMatWorldArray",matBoneSpace,MAX_BONES);
}


void CDX9SkeletalMesh::Draw()
{
	LPDIRECT3DDEVICE9 g_pd3dDevice = ((CDX9Device *)device)->g_pd3dDevice;
	// Seteo el index y el vertex buffer
	g_pd3dDevice->SetStreamSource( 0, m_vertexBuffer, 0, sizeof(SKELETAL_MESH_VERTEX));
	g_pd3dDevice->SetIndices(m_indexBuffer);

	// Seteo el vertex declaration
	SetVertexDeclaration();
	// Seteo los shaders (effect tecnica)
	SetShaders();

	// dibujo cada subset
	for(int i=0;i<cant_layers;++i)
		DrawSubset(i);
}

void CDX9SkeletalMesh::DrawSubset(int i)
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


bool CDX9SkeletalMesh::CreateFromXMLData()
{
	CDX9Device *p_device = (CDX9Device *)device;

	// create the vertex buffer
	UINT size = sizeof(SKELETAL_MESH_VERTEX) * cant_vertices;
	if( FAILED( p_device->g_pd3dDevice->CreateVertexBuffer( size, 0 , 
		0 , D3DPOOL_DEFAULT, &m_vertexBuffer, NULL ) ) )
		return false;

	// copy the vertices into the buffer
	SKELETAL_MESH_VERTEX *pVertices;
	if( FAILED( m_vertexBuffer->Lock( 0, size, (void**)&pVertices, 0 ) ) )
		return false;

	// Aprovecho para computar el tamaño y la posicion del mesh
	D3DXVECTOR3 min = D3DXVECTOR3 (10000,10000,10000);
	D3DXVECTOR3 max = D3DXVECTOR3 (-10000,-10000,-10000);

	verticesWeights = new vertexWeight[cant_vertices];
	for(int i=0;i<cant_vertices;++i)
	{
		int index = coordinatesIdx[i] * 3;
		pVertices[i].position.x = vertices[index];
		pVertices[i].position.y = vertices[index + 1];
		pVertices[i].position.z = vertices[index + 2];


		// ojo, las normales van una por coordenada id 
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


		verticesWeights[i] = aux_verticesWeights[index];

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
	layers[0].start_index = 0;
	layers[0].cant_indices = cant_vertices;			// cant_indices == cant_vertices 
	m_indexBuffer->Unlock();

	return true;
}


