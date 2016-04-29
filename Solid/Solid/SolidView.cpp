
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
#include "/dev/graphics/SkeletalMesh.h"

CRenderEngine escena;
CSkeletalMesh *p_robot = NULL;

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
	ON_COMMAND(ID_CARGAR_TGCMESH, &CSolidView::OnCargarTgcmesh)
	ON_COMMAND(ID_CARGAR_TGCSCENE, &CSolidView::OnCargarTgcScene)
	ON_COMMAND(ID_PGDN, &CSolidView::OnPgdn)
	ON_COMMAND(ID_PGUP, &CSolidView::OnPgup)
END_MESSAGE_MAP()

// Construcción o destrucción de CSolidView

CSolidView::CSolidView()
{
	eventoInterno = EV_NADA;
	primera_vez = TRUE;
}

CSolidView::~CSolidView()
{
	escena.Release();
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
		RenderLoop();
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
		escena.Release();
	escena.Create(m_hWnd);

	// Cargo un mesh de ejemplo
	int nro_obj = escena.LoadMesh("C:/TgcViewer/Examples/Media/SkeletalAnimations/BasicHuman/WomanJeans-TgcSkeletalMesh.xml");
	if(nro_obj!=-1)
	{
		p_robot = (CSkeletalMesh *)escena.m_mesh[nro_obj];
		p_robot->initAnimation(0,true,30);
	}
	scaleToFit();
}


void RenderScene()
{
	if(p_robot!=NULL)
	{
		// mesh con una animacion
		//p_robot->Render(D3DXVECTOR3(0,0,0),p_robot->m_size*100);
		p_robot->Render();
	}
	else
	{
		for(int i=0;i<escena.cant_mesh;++i)
		{
			CMesh *p_mesh = escena.m_mesh[i];
			p_mesh->Render();
		}
	}
}

void CSolidView::RenderLoop()
{
	BOOL seguir = TRUE;
	float time = 0;			
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
		T0 = T1;
		time += elapsed_time;
		escena.Update(elapsed_time);
		if(p_robot)
			p_robot->updateAnimation();
		escena.RenderFrame(RenderScene);

		D3DXVECTOR3 ViewDir = escena.lookAt - escena.lookFrom;
		D3DXVec3Normalize(&ViewDir,&ViewDir);
		MSG Msg;
		ZeroMemory( &Msg, sizeof(Msg) );
		if(PeekMessage( &Msg, NULL, 0U, 0U, PM_REMOVE ) )
		{

			if(Msg.message == WM_QUIT || Msg.message == WM_CLOSE)
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
				
				case 'W':
					escena.lookFrom += ViewDir*100;
					escena.lookAt += ViewDir;
					break;

				case 'Z':
					escena.lookFrom -= ViewDir*100;
					escena.lookAt += ViewDir;
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
				rotar_xz(&escena.lookFrom , an);
				escena.lookFrom.y += 2*dy*mxp;
			}
			break;

		case EV_PAN_REALTIME:
			{
				int dx = point.x-x0;
				int dy = point.y-y0;
				x0 = point.x;
				y0 = point.y;
				D3DXVECTOR3 VUP = D3DXVECTOR3 (0,1,0);
				D3DXVECTOR3 ViewDir = escena.lookAt - escena.lookFrom;
				D3DXVec3Normalize(&ViewDir,&ViewDir);
				D3DXVECTOR3 n;
				D3DXVec3Cross(&n,&ViewDir , &VUP);
				D3DXVec3Normalize(&n,&n);
				D3DXVECTOR3 up;
				D3DXVec3Cross(&up,&n, &ViewDir);
				D3DXVECTOR3 desf = up*(dy*mxp) + n*(dx*mxp);
				escena.lookAt += desf;
				escena.lookFrom += desf;
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
	double dist = D3DXVec3Length( &(escena.lookFrom - escena.lookAt));
	D3DXVECTOR3 v = escena.lookFrom -escena.lookAt;
	v.y = 0;
	D3DXVec3Normalize(&v,&v);
	escena.lookFrom -=   v*(zDelta/120.0 * dist * 0.1f);
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
		CString fname(ofn.lpstrFile);
		fname.Replace('\\','/');
		p_robot = NULL;
		int nro_mesh = escena.LoadMesh((char *)(LPCSTR)fname);
		if(nro_mesh!=-1)
		{
			if(IsSkeletalMesh((char *)(LPCSTR)fname))
			{
				p_robot = (CSkeletalMesh *)escena.m_mesh[nro_mesh];
				p_robot->initAnimation(0,true,30);
			}
		}
		scaleToFit();
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
		CString fname(ofn.lpstrFile);
		fname.Replace('\\','/');
		p_robot = NULL;
		escena.LoadSceneFromXml((char *)(LPCSTR)fname);
		scaleToFit();
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



// ajusta el pto de vista para que el mesh sea visible 
void CSolidView::scaleToFit()
{
	D3DXVECTOR3 min = D3DXVECTOR3 (10000,10000,10000);
	D3DXVECTOR3 max = D3DXVECTOR3 (-10000,-10000,-10000);
	for(int i=0;i<escena.cant_mesh;++i)
	{
		D3DXVECTOR3 p0 = escena.m_mesh[i]->m_pos;
		if(p0.x<min.x)
			min.x = p0.x;
		if(p0.y<min.y)
			min.y = p0.y;
		if(p0.z<min.z)
			min.z = p0.z;


		D3DXVECTOR3 p1 = p0 + escena.m_mesh[i]->m_size;
		if(p1.x>max.x)
			max.x = p1.x;
		if(p1.y>max.y)
			max.y = p1.y;
		if(p1.z>max.z)
			max.z = p1.z;
	}

	escena.lookFrom = escena.lookAt = (min + max) * 0.5;
	escena.lookFrom.x += D3DXVec3Length(&(max - min)) * 2;

}


