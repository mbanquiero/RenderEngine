#include "stdafx.h"
#include "/dev/graphics/RenderEngine.h"
#include "/dev/graphics/dx11device.h"
#include "/dev/graphics/dx9device.h"

CRenderEngine::CRenderEngine()
{
	device = NULL;
	cur_camera = NULL;
	cant_texturas = 0;
	cant_mesh = 0;
	cant_obj = 0;
	total_time = 0;
	elpased_time = 0;
}

CRenderEngine::~CRenderEngine()
{
	if(device!=NULL)
		CleanD3D();
}


bool CRenderEngine::IsReady()
{
	return device!=NULL ? true : false;
}

void CRenderEngine::Create(HWND hwnd,int version)
{
	if(version==0)
		device = new CDX11Device();
	else
		device = new CDX9Device();

	InitD3D(hwnd);
	InitPipeline();
	InitGraphics();
}



// Inicializa el DirectX
void  CRenderEngine::InitD3D(HWND hWnd)
{
	device->InitD3D(hWnd);

}

void CRenderEngine::ClearScene()
{
	ReleaseTextures();
	ReleaseMeshes();
	ReleaseObj();
}

void CRenderEngine::CleanD3D()
{
	ClearScene();
	device->CleanD3D();

	if(device!=NULL)
	{
		delete device;
		device = NULL;
	}

}

void CRenderEngine::ReleaseTextures()
{
	for(int i=0;i<cant_texturas;++i)
	{
		delete m_texture[i];
		m_texture[i] = NULL;
	}
	cant_texturas = 0;
}

void CRenderEngine::InitPipeline()
{
	device->InitPipeline();
}

void CRenderEngine::InitGraphics()
{
	// Camera
	if(cur_camera==NULL)
		// si no tiene puesta ninguna camera, le dejo la camara rotacional x defecto
		cur_camera = &rot_camera;

	cur_camera->LF = Vector3(0.0f, 0.0f, -10.0f);

	// Matrices de Transformacion
	device->InitTransformPipeline();
}

void CRenderEngine::Update(float p_elpased_time)
{
	elpased_time = p_elpased_time;
	total_time += elpased_time;
}

void CRenderEngine::RenderFrame()
{
	// Actualizo la camara
	cur_camera->Update();
	// Preparo el device para renderizar la escena
	device->BeginRenderFrame(cur_camera->m_viewMatrix);

	// dibujo los objetos graficos pp dichos
	for(int i=0;i<cant_obj;++i)
		m_obj[i]->Render();

	// Finalizo el device
	device->EndRenderFrame();
}


// Transform pipeline
void CRenderEngine::SetTransformWorld(D3DXMATRIX matWorld)
{
	device->m_World = matWorld;
}

void CRenderEngine::SetTransformView(D3DXMATRIX matView)
{
	device->m_View = matView;
}

void CRenderEngine::SetTransformProj(D3DXMATRIX matProj)
{
	device->m_Proj = matProj;
}

void CRenderEngine::SetMatrixBuffer()
{
	device->SetShaderTransform((D3DXVECTOR3)cur_camera->LF);
}


int CRenderEngine::LoadTexture(char *filename)
{
	// primero busco si la textura ya esta cargada
	int rta = -1;
	for(int i=0;i<cant_texturas && rta==-1;++i)
		if(strcmp(filename,m_texture[i]->name)==0)
			rta = i;

	if(rta!=-1)
		return rta;			// La textura ya estaba cargada en el pool, devuelve el nro de textura 

	// Carga la textura pp dicha
	// Primero verifico que este en la carpeta que me dice
	char fname[MAX_PATH];
	strcpy(fname,filename);
	if(!is_file(fname))
	{
		// Busco en la carpeta media\texturas
		sprintf(fname,"c:\\dev\\media\\texturas\\%s",filename);
		if(!is_file(fname))
		{
			// pruebo con la extension jpg
			extension(fname,"jpg");
			if(!is_file(fname))
			{
				// pruebo con la extension png
				extension(fname,"png");
				if(!is_file(fname))
				{
					// pruebo con la extension bmp
					extension(fname,"bmp");
				}
			}
		}
	}
	
	CBaseTexture *p_texture = device->CreateTexture(fname);
	if(p_texture==NULL)
		return -1;
	m_texture[cant_texturas] = p_texture;
	return cant_texturas++;
}

int CRenderEngine::LoadMesh(char *filename)
{
	// primero busco si la textura ya esta cargada
	int rta = -1;
	for(int i=0;i<cant_mesh&& rta==-1;++i)
		if(strcmp(filename,m_mesh[i]->fname)==0)
			rta = i;

	if(rta!=-1)
		return rta;			// el mesh ya estaba cargada en el pool, devuelve el nro de textura 

	// Carga el mesh pp dicho
	// Le pido al device que cargue el mesh 
	CBaseMesh *p_mesh = device->LoadMesh(this,filename);
	if(p_mesh!=NULL)
		m_mesh[rta = cant_mesh++] = p_mesh;

	return rta;
}

void CRenderEngine::ReleaseMeshes()
{
	for(int i=0;i<cant_mesh;++i)
	{
		delete m_mesh[i];
		m_mesh[i] = NULL;
	}
	cant_mesh = 0;
}

void CRenderEngine::ReleaseObj()
{
	for(int i=0;i<cant_obj;++i)
	{
		delete m_obj[i];
		m_obj[i] = NULL;
	}
	cant_obj = 0;
}

CGraphicObject *CRenderEngine::CreateMesh(char *filename,D3DXVECTOR3 pos,D3DXVECTOR3 size,D3DXVECTOR3 rot)
{
	// Cargo el mesh pp dicho
	int nro_mesh = LoadMesh(filename);
	if(nro_mesh==-1)
		return NULL;		// Error

	if(size.x + size.y + size.z <0.1)
		size = m_mesh[nro_mesh]->m_size;

	CGraphicObject *p_obj = new CGraphicObject();
	p_obj->M = this;
	p_obj->nro_mesh = nro_mesh;
	p_obj->tipo = 0;
	p_obj->m_pos = pos;
	p_obj->m_size = size;
	p_obj->m_rot = rot;

	// Aprovecho para calcular la matriz de world de este objeto
	p_obj->CalcularMatriz(device->transponer_matrices);

	// Asigno el obj a la lista de objetos
	m_obj[cant_obj++] = p_obj;

	// Devuelvo el puntero al objeto
	return 	p_obj;
}


// helper
char is_file(char *file_name)
{
	if(file_name==NULL || esta_vacio(file_name))
		return 0;

	FILE *fp;
	char rta=1;
	if((fp=fopen(file_name,"rt"))==NULL)
		rta=0;
	else
		fclose(fp);
	return(rta); 
}

// devuelve TRUE si el string esta vacio,
// es decir si es "", o bien solo tiene espacios
bool esta_vacio(char *s)
{
	bool rta;
	if(s[0]==0)
		rta = true;		// se trata de ""
	else
	if(s[0]!=32)
		rta = false;	// no esta vacio
	else
	{
		// empieza en espacios, pero puede llegar a 
		// tener algo en el medio
		rta = true;		// supongo que esta vacio
		int i=0;
		while(s[i] && rta)
			if(s[i]!=32)
				rta = false;		// no esta vacio
			else
				++i;
	}
	return rta;
}

void extension(char *file,char *ext)
{
	int i=0;

	CString sFileName = (CString)file;
	// Le saco la extension que tenia antes
	// si es que tiene extension
	if(sFileName.Find('.')!=-1)
		sFileName = sFileName.Left(sFileName.ReverseFind('.'));

	if(ext && ext[0])
	{
		// Ahora no tiene mas extension, le agrego la nueva
		sFileName+=".";
		sFileName+=ext;
	}

	// y lo almaceno en *file 
	strcpy(file,(LPCSTR)sFileName);
}

char *rtrim(char *string)
{
	int l=strlen(string)-1;
	while(l>=0 && (*(string+l)==' ' || *(string+l)=='\t'))
	{
		*(string+l)='\0';
		--l;
	}
	return(string);
}

char *ltrim(char *string)
{
	int l = strlen(string);
	char *buffer = new char[l+1];
	int i=0;
	while(string[i]==' ' || string[i]=='\t')
		++i;
	strcpy(buffer,string+i);
	strcpy(string,buffer);
	delete buffer;
	return(string);
}

char *que_extension(char *file,char *ext)
{
	int t =0;
	// busco el punto (empiezando desde atras)
	char *p = strrchr(file,'.');
	if(p!=NULL)
	{
		// salteo el punto
		++p;
		while(*p!=NULL)
			ext[t++] = *p++;
	}
	ext[t] = '\0';
	return ext;
}


/*
// Devuelve que obj esta sobre ese punto
Vector3 CRenderEngine::que_punto(CPoint p)
{
	D3D11_VIEWPORT viewport11;
	ZeroMemory(&viewport11, sizeof(D3D11_VIEWPORT));
	UINT cant_viewports;
	devcon->RSGetViewports(&cant_viewports, &viewport11);
	D3D10_VIEWPORT viewport;
	viewport.TopLeftX = viewport11.TopLeftX;
	viewport.TopLeftY = viewport11.TopLeftY;
	viewport.Width = viewport11.Width;
	viewport.Height = viewport11.Height;
	viewport.MinDepth = viewport11.MinDepth;
	viewport.MaxDepth = viewport11.MaxDepth;

	// un punto de pantalla xy, le correspen infinitos puntos del espacio xyz de la escena
	// tomo 2 puntos, uno para z=0 y otro z=1, y la recta que une esos 2 puntos
	// es el conjunto de puntos para los cuales la proyeccion = xy
	D3DXVECTOR3 pV0(p.x,p.y,0);			// proyeccion del punto sobre el plano mas cercano (z=0)
	D3DXVECTOR3 pV1(p.x,p.y,1);			// proyeccion del punto sobre el plano mas lejano (z=1)
	D3DXVECTOR3 pt0,pt1;
	D3DXVec3Unproject(&pt0,&pV0,&viewport,&m_Proj,&m_View,&m_World);
	D3DXVec3Unproject(&pt1,&pV1,&viewport,&m_Proj,&m_View,&m_World);

	// voy a intersectar esta recta con todos los faces, y me voy a quedar con el mas cercano
	// al punto de vista (z min)
	Vector3 P0 = Vector3(pt0.x,pt0.y,pt0.z);
	Vector3 P1 = Vector3(pt1.x,pt1.y,pt1.z);
	Vector3 d = P1-P0;
	d.normalizar();
	D3DXVECTOR3 pRayPos =  (D3DXVECTOR3)P0;
	D3DXVECTOR3 pRayDir = (D3DXVECTOR3)d;

	double z_min = 1;
	int rta = -1;

	for(int i=0;i<cant_obj;++i)
	{
		Vector3 Ip,N;
			DWORD aux_face; 
			if(pobj->mesh!=-1 && Mesh[pobj->mesh].interseccion(pobj,P0,d,&Ip,&N,&aux_face))
			{
				D3DXVECTOR3 pV(Ip.x,Ip.y,Ip.z);
				D3DXVECTOR3 pt;
				D3DXVec3Project(&pt,&pV,&viewport,&matProj,&matView,&matWorld);
				if(pt.z<z_min)
				{
					z_min = pt.z;
					rta = i;
					face = aux_face;

					// devuelve el pto de interseccion y la normal
					*Ip0 = Ip;		
					*Normal = N;		
				}
			}
		}
	}

	return Vector3(0,0,0);
}
*/

// Soporte de archivos xml
#define BUFFER_SIZE  600000
D3DXVECTOR3 ParserXMLVector3(char *buffer);

bool CRenderEngine::LoadSceneFromXml(char *filename)
{
	FILE *fp = fopen(filename,"rt");
	if(fp==NULL)
		return 0;

	// Leo y parseo el xml
	int cant = 0;
	char mesh_id[MAX_MESH][255];
	memset(mesh_id,0,sizeof(mesh_id));
	D3DXVECTOR3 mesh_pos[MAX_MESH];

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
				D3DXVECTOR3 pos = ParserXMLVector3(p+6);
				mesh_pos[cant].x = pos.x;
				mesh_pos[cant].y = pos.z;
				mesh_pos[cant].z = pos.y;
				
			}

			// tomo el nombre del mesh
			p = strchr(buffer+12,'\'');
			if(p!=NULL)
			{
				*p = 0;
				strcpy(mesh_id[cant],buffer+12);

				++cant;
			}
		}
	}
	delete buffer,
	fclose(fp);

	// Ahora tengo que cargar los mesh pp dichos.
	ClearScene();

	for(int i=0;i<cant;++i)
	{
		// Cargo el mesh 
		CBaseMesh *p_mesh = device->LoadMeshFromXmlFile(this,filename,mesh_id[i]);
		if(p_mesh!=NULL)
		{
			// Corrijo el file name
			strcat(p_mesh->fname," - ");
			strcat(p_mesh->fname,mesh_id[i]);
			m_mesh[cant_mesh] = p_mesh;

			CGraphicObject *p_obj = new CGraphicObject();
			p_obj->M = this;
			p_obj->nro_mesh = cant_mesh;
			p_obj->tipo = 0;
			p_obj->m_pos = mesh_pos[i] - p_mesh->m_size*0.5;
			p_obj->m_size = p_mesh->m_size;
			p_obj->m_rot = D3DXVECTOR3(0,0,0);
			// Aprovecho para calcular la matriz de world de este objeto
			p_obj->CalcularMatriz(device->transponer_matrices);
			// Asigno el obj a la lista de objetos
			m_obj[cant_obj++] = p_obj;
			// indico que hay un mesh mas en la lista de meshes
			++cant_mesh;
		}
	}
	// si todo salio bien tiene que haber un objeto por mesh y tantos mesh como cantidad queriamos cargar
	return cant_obj==cant_mesh && cant_obj==cant?true:false;
}