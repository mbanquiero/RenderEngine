#include "stdafx.h"
#include "/dev/graphics/mesh.h"
#include "/dev/graphics/SkeletalMesh.h"
#include "/dev/graphics/RenderEngine.h"
#include "/dev/graphics/xstring.h"
#include "/dev/graphics/TGCViewer.h"

#define BUFFER_SIZE  600000

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


CSkeletalMesh::CSkeletalMesh() : CMesh()
{
	verticesWeights = NULL;
	cant_animaciones = 0;

	m_vertexBuffer = NULL;
	m_indexBuffer = NULL;

	pVertices = NULL;

}

CSkeletalMesh::~CSkeletalMesh()
{
	SAFE_DELETE(verticesWeights);
	for(int i=0;i<cant_animaciones;++i)
		SAFE_DELETE(animacion[i]);
	ReleaseInternalData();
	Release();
}


void CSkeletalMesh::Release()
{
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
}

void CSkeletalMesh::ReleaseInternalData()
{
	if(pVertices!=NULL)
	{
		delete []pVertices;
		pVertices = NULL;
	}
	CMesh::ReleaseInternalData();
}


bool CSkeletalMesh::LoadFromXMLFile(CRenderEngine *p_engine,char *filename)
{

	CTGCSkeletalMeshParser loader;
	if(!loader.LoadSkeletalMesh(this,filename))
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



bool CSkeletalMesh::LoadAnimation(char *fname)
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
		ParseXMLLine(buffer);
	}
	fclose(fp);
	return true;
}


bool CSkeletalAnimation::ParseXMLLine(char *buffer)
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
			bone_animation[bone_id].frame[n].Position = CTGCXmlParser::ParseXMLVector3(p+6);;
		}					

		// Orientacion
		p = strstr(buffer , "rotQuat='");
		if(p!=NULL)
		{
			D3DXVECTOR4 rot = CTGCXmlParser::ParseXMLVector4(p+10);
			bone_animation[bone_id].frame[n].Rotation.x = rot.x;
			bone_animation[bone_id].frame[n].Rotation.y = rot.y;
			bone_animation[bone_id].frame[n].Rotation.z = rot.z;
			bone_animation[bone_id].frame[n].Rotation.w = rot.w;
		}					
	}
	return procesada;
}




void CSkeletalMesh::setupSkeleton()
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

void CSkeletalMesh::initAnimation(int nro_animacion, bool con_loop, float userFrameRate)
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
void CSkeletalMesh::updateMeshVertices()
{
	//Precalcular la multiplicación para llevar a un vertice a Bone-Space y luego transformarlo segun el hueso
	//Estas matrices se envian luego al Vertex Shader para hacer skinning en GPU
	for (int i = 0; i < cant_bones; i++)
		matBoneSpace[i] = bones[i].matInversePose * bones[i].matFinal;
}



/// Actualiza el cuadro actual de la animacion.
void CSkeletalMesh::updateAnimation()
{
	//Ver que haya transcurrido cierta cantidad de tiempo
	if(engine->elapsed_time < 0)
		return;

	//Sumo el tiempo transcurrido
	currentTime += engine->elapsed_time;

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
void CSkeletalMesh::updateSkeleton()
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
int CSkeletalMesh::getCurrentFrameBone(st_bone_animation *boneFrames, float currentFrame)
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




void CSkeletalMesh::SetVertexDeclaration()
{
	engine->g_pd3dDevice->SetVertexDeclaration(engine->m_pSkeletalMeshVertexDeclaration);
	bpv = sizeof(SKELETAL_MESH_VERTEX);
}

// set the shader objects
void CSkeletalMesh::SetShaders()
{
	LPDIRECT3DDEVICE9 g_pd3dDevice = engine->g_pd3dDevice;
	ID3DXEffect *g_pEffect = engine->g_pEffectStandard;
	g_pEffect->SetTechnique("SkeletalRender");
	g_pEffect->SetMatrixArray("bonesMatWorldArray",matBoneSpace,MAX_BONES);
}


void CSkeletalMesh::Draw()
{
	LPDIRECT3DDEVICE9 g_pd3dDevice = engine->g_pd3dDevice;
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

void CSkeletalMesh::DrawSubset(int i)
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


bool CSkeletalMesh::CreateMeshFromData(CRenderEngine *p_engine)
{
	// Cargo las distintas texturas en el engine, y asocio el nro de textura en el layer del mesh
	for(int i=0;i<cant_layers;++i)
	{
		// Cargo la textura en el pool (o obtengo el nro de textura si es que ya estaba)
		layers[i].nro_textura = p_engine->LoadTexture(layers[i].texture_name);
	}

	// create the vertex buffer
	UINT size = sizeof(SKELETAL_MESH_VERTEX) * cant_vertices;
	if( FAILED( p_engine->g_pd3dDevice->CreateVertexBuffer( size, 0 , 
		0 , D3DPOOL_DEFAULT, &m_vertexBuffer, NULL ) ) )
		return false;

	SKELETAL_MESH_VERTEX *p_gpu_vb;
	if( FAILED( m_vertexBuffer->Lock( 0, size, (void**)&p_gpu_vb, 0 ) ) )
		return false;
	memcpy(p_gpu_vb,pVertices,size);
	m_vertexBuffer->Unlock();


	// Index buffer
	size = sizeof(unsigned long) * cant_indices;
	if( FAILED( p_engine->g_pd3dDevice->CreateIndexBuffer( size, 0 ,D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_indexBuffer, NULL ) ) )
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

