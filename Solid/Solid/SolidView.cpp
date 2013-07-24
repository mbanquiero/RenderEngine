
// SolidView.cpp: implementación de la clase CSolidView
//

#include "stdafx.h"
// Se pueden definir SHARED_HANDLERS en un proyecto ATL implementando controladores de vista previa, miniatura
// y filtro de búsqueda, y permiten compartir código de documentos con ese proyecto.
#ifndef SHARED_HANDLERS
#include "Solid.h"
#endif

#include "SolidDoc.h"
#include "SolidView.h"

#include "/dev/graphics/RenderEngine.h"
#include "/dev/graphics/vectores.h"
#include "/dev/graphics/SkeletalMesh.h"
CRenderEngine escena;
CDX11SkeletalMesh *p_robot = NULL;
	//, escena2;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CSolidView

IMPLEMENT_DYNCREATE(CSolidView, CView)

BEGIN_MESSAGE_MAP(CSolidView, CView)
	// Comandos de impresión estándar
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSolidView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_COMMAND(ID_DEVICE_DIRECTX11, &CSolidView::OnDeviceDirectx11)
	ON_COMMAND(ID_DEVICE_DIRECTX9, &CSolidView::OnDeviceDirectx9)
	ON_COMMAND(ID_CARGAR_TGCMESH, &CSolidView::OnCargarTgcmesh)
	ON_COMMAND(ID_CARGAR_TGCSCENE, &CSolidView::OnCargarTgcScene)
	ON_COMMAND(ID_PGDN, &CSolidView::OnPgdn)
	ON_COMMAND(ID_PGUP, &CSolidView::OnPgup)
END_MESSAGE_MAP()

// Construcción o destrucción de CSolidView

CSolidView::CSolidView()
{
	eventoInterno = EV_NADA;
	version_dx = DEVICE_DX11;
}

CSolidView::~CSolidView()
{
	escena.CleanD3D();
}

BOOL CSolidView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: modificar aquí la clase Window o los estilos cambiando
	//  CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// Dibujo de CSolidView

void CSolidView::OnDraw(CDC* /*pDC*/)
{
	CSolidDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if(primera_vez )
	{
		primera_vez = false;
		QueryFPS();
	}
}


// Impresión de CSolidView


void CSolidView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CSolidView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Preparación predeterminada
	return DoPreparePrinting(pInfo);
}

void CSolidView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: agregar inicialización adicional antes de imprimir
}

void CSolidView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: agregar limpieza después de imprimir
}

void CSolidView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CSolidView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// Diagnósticos de CSolidView

#ifdef _DEBUG
void CSolidView::AssertValid() const
{
	CView::AssertValid();
}

void CSolidView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSolidDoc* CSolidView::GetDocument() const // La versión de no depuración es en línea
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSolidDoc)));
	return (CSolidDoc*)m_pDocument;
}
#endif //_DEBUG


// Controladores de mensaje de CSolidView

void CSolidView::ArmarEscena()
{
	if(escena.IsReady())
		escena.CleanD3D();
	escena.Create(m_hWnd,version_dx);
	//escena.LoadTexture("/dev/media/cerezo.jpg");
	//escena.LoadTexture("/dev/media/tecid57.jpg");
	//for(int i = 0; i< 20 ; ++i)
	//	escena.CreateMesh("/dev/media/PERSONA003.y",D3DXVECTOR3(-4000 + i*400,1700/2,0),D3DXVECTOR3(500,1700,700));
	//escena.CreateMesh("/dev/media/piso.y",D3DXVECTOR3(0,-100,0),D3DXVECTOR3(10000,100,10000));
	if(0)
	{
		// Ejemplo mesh camion
		escena.CreateMesh("C:/TgcViewer/Examples/Media/MeshCreator/Meshes/Vehiculos/CamionDeAgua/CamionDeAgua-TgcScene.xml",
			D3DXVECTOR3(0,-100,0),D3DXVECTOR3(900,1800,600));
	}
	else
	{
		// ejemplo robot
		//CGraphicObject *p_obj = escena.CreateMesh("C:/TgcViewer/Examples/Media/SkeletalAnimations/Robot/Robot-TgcSkeletalMesh.xml",D3DXVECTOR3(0,-100,0));
		CGraphicObject *p_obj = escena.CreateMesh("C:/TgcViewer/Examples/Media/SkeletalAnimations/BasicHuman/WomanJeans-TgcSkeletalMesh.xml",D3DXVECTOR3(0,-100,0));
		if(p_obj!=NULL)
		{
			p_obj->Scale(50);
			//TODO: Desglozar la malla del objeto pp dicho en la animacion
			p_robot = (CDX11SkeletalMesh *)escena.m_mesh[p_obj->nro_mesh];
			p_robot->initAnimation(0,true,30);
		}
	}
	escena.rot_camera.LF = Vector3(300, 4000, -4500);
	escena.rot_camera.LA = Vector3(0, 500, 0);

	/*
	escena2.Create(wnd_2.m_hWnd,1);
	escena2.LoadTexture("/dev/media/cerezo.jpg");
	escena2.LoadTexture("/dev/media/tecid57.jpg");
	for(int i = 0; i< 20 ; ++i)
		escena2.CreateMesh("/dev/media/PERSONA003.y",D3DXVECTOR3(-4000 + i*400,1700/2,0),D3DXVECTOR3(500,1700,700));
	escena2.CreateMesh("/dev/media/piso.y",D3DXVECTOR3(0,-100,0),D3DXVECTOR3(10000,100,10000));
	escena2.rot_camera.LF = Vector3(300, 4000, -4500);
	escena2.rot_camera.LA = Vector3(0, 500, 0);
	*/

}

void CSolidView::QueryFPS()
{
	BOOL seguir = TRUE;
	int cant_frames = 0;
	float time = 0;			
	float ftime = 0;
	float fps = 0;
	LARGE_INTEGER F,T0,T1;   // address of current frequency
	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);

	if(!escena.IsReady())
	{
		ArmarEscena();
	}

	while(seguir)
	{

		QueryPerformanceCounter(&T1);
		double elapsed_time = (double)(T1.QuadPart - T0.QuadPart) / (double)F.QuadPart;
		ftime += elapsed_time;
		time += elapsed_time;
		T0 = T1;
		++cant_frames;

		escena.Update(elapsed_time);
		if(p_robot)
			p_robot->updateAnimation();
		escena.RenderFrame();

		if(ftime>1)
		{
			fps = cant_frames/ftime;
			ftime = 0;
			cant_frames = 0;
		}

		MSG Msg;
		ZeroMemory( &Msg, sizeof(Msg) );
		if(PeekMessage( &Msg, NULL, 0U, 0U, PM_REMOVE ) )
		{

			if(Msg.message == WM_QUIT)
			{
				seguir = FALSE;
				break;
			}

			// dejo que windows procese el mensaje
			TranslateMessage( &Msg );
			DispatchMessage( &Msg );

			switch(Msg.message)
			{
			case WM_KEYDOWN:
				switch((int) Msg.wParam)	    // virtual-key code 
				{
				case VK_NEXT:
					OnPgdn();
					break;
				case VK_PRIOR:
					OnPgup();
					break;
				case VK_ESCAPE:
					seguir = FALSE;
					break;

				case '1':
					if(escena.cant_obj)
					{
						// Agrandar objeto
						CGraphicObject *p_obj = escena.m_obj[0];
						p_obj->Scale(1.1);
					}
					break;

				case '2':
					if(escena.cant_obj)
					{
						// achicar objeto
						CGraphicObject *p_obj = escena.m_obj[0];
						p_obj->Scale(0.9);
					}
					break;

				}
				break;
			}

		}
	}
}



void CSolidView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// almaceno la posicion inical del mouse
	x0 = point.x;
	y0 = point.y;
	SetCapture();
	eventoInterno = EV_3D_REALTIME;

	CView::OnLButtonDown(nFlags, point);
}


void CSolidView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(point.x==pos_cursor.x && point.y==pos_cursor.y)
		return;				// bug del windows que llama a onmousemove, aun cuando el mouse no se esta moviendo
	pos_cursor = point;
	CView::OnMouseMove(nFlags, point);

	float mxp = 10;				// milimetros x pixel
	switch(eventoInterno)
	{
		case EV_3D_REALTIME:
			{
				// la camara vuelo de pajaro y las camas auxiliares no permiten
				// rotar la escena, eso debido a que setean un farplane ajustado
				// y al rotar se produce el artifact
				int dx = point.x-x0;
				int dy = point.y-y0;
				x0 = point.x;
				y0 = point.y;
				double tot_x = 800;
				double an = dx/tot_x*2*3.1415;
				// uso el desplazamiento vertical, par elevar o bajar el punto de vista
				//escena.LF.z += 2*dy/ey;
				// uso el desplazamiento en x para rotar el punto de vista  en el plano xy
				if(nFlags&MK_CONTROL)
				{

					Vector3 VUP = Vector3(0,1,0);
					Vector3 ViewDir = escena.cur_camera->LA - escena.cur_camera->LF;
					ViewDir.normalizar();

					escena.cur_camera->LF.rotar_xz(escena.cur_camera->LA,an);
					Vector3 N;
					if(fabs(ViewDir.y)<0.95)
						N = Vector3(0,1,0)*ViewDir;
					else
						N = Vector3(0,0,1)*ViewDir;
					double tot_y = 600;
					double an_y = dy/tot_y*M_PI;
					escena.cur_camera->LF.rotar(escena.cur_camera->LA,N,an_y);
				}
				else
				{
					escena.cur_camera->LF.rotar_xz(an);
					escena.cur_camera->LF.y += 2*dy*mxp;
				}
			}
			break;

		case EV_PAN_REALTIME:
			{
				int dx = point.x-x0;
				int dy = point.y-y0;
				x0 = point.x;
				y0 = point.y;
				Vector3 VUP = Vector3(0,1,0);
				Vector3 d = escena.cur_camera->LA - escena.cur_camera->LF;
				d.normalizar();
				Vector3 n = d*VUP;
				n.normalizar();
				Vector3 up = n*d;
				Vector3 desf = up*(dy*mxp) + n*(dx*mxp);
				escena.cur_camera->LA = escena.cur_camera->LA + desf;
				escena.cur_camera->LF = escena.cur_camera->LF + desf;
			}
			break;

		default:
			break;
	}
}


void CSolidView::OnLButtonUp(UINT nFlags, CPoint point)
{
	eventoInterno = EV_NADA;
	ReleaseCapture();
	CView::OnLButtonUp(nFlags, point);
}


BOOL CSolidView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// acercame o alejarme 
	CBaseCamera *p_cam = escena.cur_camera;
	double dist = (p_cam->LF - p_cam->LA).mod();
	Vector3 v = escena.cur_camera->LF-escena.cur_camera->LA;
	v.y = 0;
	v.normalizar();
	escena.cur_camera->LF = escena.cur_camera->LF - v*(zDelta/120.0 * dist * 0.1f);


	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CSolidView::OnMButtonDown(UINT nFlags, CPoint point)
{
	// almaceno la posicion inical del mouse
	x0 = point.x;
	y0 = point.y;
	eventoInterno = EV_PAN_REALTIME;
	SetCapture();
	CView::OnMButtonDown(nFlags, point);
}


void CSolidView::OnMButtonUp(UINT nFlags, CPoint point)
{
	// Termina el evento de pan realtime
	eventoInterno = EV_NADA;
	ReleaseCapture();
	CView::OnMButtonUp(nFlags, point);
}


void CSolidView::OnDeviceDirectx11()
{
	version_dx = 0;
	ArmarEscena();
}


void CSolidView::OnDeviceDirectx9()
{
	version_dx = 1;
	ArmarEscena();
}


void CSolidView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

}


void CSolidView::OnCargarTgcmesh()
{
	// TODO: Agregue aquí su código de controlador de comandos
	OPENFILENAME ofn;
	char szFile[256], szFileTitle[256];
	unsigned int i;
	char  szFilter[256];
	strcpy(szFilter,"Archivos de Mesh(*.xml)|*.xml");
	szFile[0] = '\0';
	for(i=0;szFilter[i]!='\0';++i)
		if(szFilter[i]=='|')
			szFilter[i]='\0';

	// Set all structure members to zero. 
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = AfxGetApp()->m_hInstance;
	ofn.hwndOwner = AfxGetMainWnd()->m_hWnd;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile= szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST |OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;

	if (GetOpenFileName( &ofn ))
	{
		escena.ReleaseTextures();
		escena.ReleaseMeshes();
		escena.ReleaseObj();
		CString fname(ofn.lpstrFile);
		fname.Replace('\\','/');
		p_robot = NULL;
		CGraphicObject *p_obj = escena.CreateMesh((char *)(LPCSTR)fname,D3DXVECTOR3(0,0,0));
		if(p_obj!=NULL)
		{
			p_obj->Scale(50);
			if(IsSkeletalMesh((char *)(LPCSTR)fname))
			{
				//TODO: Desglozar la malla del objeto pp dicho en la animacion
				p_robot = (CDX11SkeletalMesh *)escena.m_mesh[p_obj->nro_mesh];
				p_robot->initAnimation(0,true,30);

			}
		}
	}
}



void CSolidView::OnCargarTgcScene()
{
	OPENFILENAME ofn;
	char szFile[256], szFileTitle[256];
	unsigned int i;
	char  szFilter[256];
	strcpy(szFilter,"Archivos de Escena(*.xml)|*.xml");
	szFile[0] = '\0';
	for(i=0;szFilter[i]!='\0';++i)
		if(szFilter[i]=='|')
			szFilter[i]='\0';

	// Set all structure members to zero. 
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = AfxGetApp()->m_hInstance;
	ofn.hwndOwner = AfxGetMainWnd()->m_hWnd;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile= szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST |OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;

	if (GetOpenFileName( &ofn ))
	{
		escena.ReleaseTextures();
		escena.ReleaseMeshes();
		escena.ReleaseObj();
		CString fname(ofn.lpstrFile);
		fname.Replace('\\','/');
		p_robot = NULL;
		int cant_objetos = escena.LoadSceneFromXml((char *)(LPCSTR)fname);
	}
}


void CSolidView::OnPgdn()
{
	if(p_robot==NULL)
		return;
	int currAnimation = p_robot->currentAnimation+1;
	if(currAnimation>=p_robot->cant_animaciones)
		currAnimation = 0;
	p_robot->initAnimation(currAnimation);

}

void CSolidView::OnPgup()
{
	if(p_robot==NULL)
		return;
	int currAnimation = p_robot->currentAnimation-1;
	if(currAnimation<0)
		currAnimation = p_robot->cant_animaciones  - 0;
	p_robot->initAnimation(currAnimation);
}
