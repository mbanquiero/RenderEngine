
// ChildView.cpp: implementación de la clase CChildView
//

#include "stdafx.h"
#include "volviewer.h"
#include "ChildView.h"
#include "RenderEngine.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CRenderEngine escena;

// CChildView

CChildView::CChildView()
{
	primera_vez = true;
	ev_rotar = false;
}

CChildView::~CChildView()
{
	escena.Release();
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// Controladores de mensaje de CChildView

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // Contexto de dispositivo para dibujo

	if(primera_vez )
	{
		primera_vez = false;
		if( !escena.Initialize( dc.m_hDC))
			AfxMessageBox( "Error al iniciar opengl",MB_ICONSTOP);
		else
			RenderLoop();
	}

}


// helper clamp pos
float clamp256(float x)
{
	if(x<-128)
		x+=256;
	else
	if(x>128)
		x-=256;
	return x;
}


void CChildView::RenderLoop()
{
	BOOL seguir = TRUE;
	float time = 0;			
	LARGE_INTEGER F,T0,T1;   // address of current frequency
	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);

	int cant_frames = 0;
	float frame_time = 0;

	while(seguir)
	{

		QueryPerformanceCounter(&T1);
		double elapsed_time = (double)(T1.QuadPart - T0.QuadPart) / (double)F.QuadPart;
		T0 = T1;
		time += elapsed_time;
		frame_time += elapsed_time;
		escena.time = time;
		escena.elapsed_time = elapsed_time;
		if(frame_time>1)
		{
			escena.fps = cant_frames/frame_time;
			frame_time = 0;
			cant_frames = 0;
		}
		

		float vel_rot = elapsed_time*1.5;
		vec3 cero = vec3(0,0,0);

		if(GetAsyncKeyState(VK_RIGHT))
		{
			escena.lookFrom.rotar(cero,escena.U,-vel_rot);
			escena.V.rotar(cero,escena.U,-vel_rot);
		}
		if(GetAsyncKeyState(VK_LEFT))
		{
			escena.lookFrom.rotar(cero,escena.U,vel_rot);
			escena.V.rotar(cero,escena.U,vel_rot);
		}


		if(GetAsyncKeyState(VK_UP))
		{
			escena.lookFrom.rotar(cero,escena.V,vel_rot);
			escena.U.rotar(cero,escena.V,vel_rot);
		}

		if(GetAsyncKeyState(VK_DOWN))
		{
			escena.lookFrom.rotar(cero,escena.V,-vel_rot);
			escena.U.rotar(cero,escena.V,-vel_rot);
		}



		/*
		if(GetAsyncKeyState(VK_RIGHT))
		{
			escena.viewDir.rotar(cero,escena.U,-vel_rot);
			escena.V.rotar(cero,escena.U,-vel_rot);
		}
		if(GetAsyncKeyState(VK_LEFT))
		{
			escena.viewDir.rotar(cero,escena.U,vel_rot);
			escena.V.rotar(cero,escena.U,vel_rot);
		}

		if(GetAsyncKeyState(VK_UP))
		{
			escena.viewDir.rotar(cero,escena.V,vel_rot);
			escena.U.rotar(cero,escena.V,vel_rot);
		}
			
		if(GetAsyncKeyState(VK_DOWN))
		{
			escena.viewDir.rotar(cero,escena.V,-vel_rot);
			escena.U.rotar(cero,escena.V,-vel_rot);
		}*/


		/*
		vec3 Up = vec3(0,1,0);
		vec3 N = vec3(0,0,1);
		vec3 BN = vec3(1,0,0);
		if(GetAsyncKeyState(VK_RIGHT))
		{
			escena.viewDir.rotar(cero,Up,-vel_rot);
			escena.V.rotar(cero,Up,-vel_rot);
			escena.U.rotar(cero,Up,-vel_rot);
		}
		if(GetAsyncKeyState(VK_LEFT))
		{
			escena.viewDir.rotar(cero,Up,vel_rot);
			escena.V.rotar(cero,Up,vel_rot);
			escena.U.rotar(cero,Up,vel_rot);
		}


		if(GetAsyncKeyState(VK_SHIFT))
		{
			if(GetAsyncKeyState(VK_UP))
			{
				escena.viewDir.rotar(cero,N,-vel_rot);
				escena.V.rotar(cero,N,-vel_rot);
				escena.U.rotar(cero,N,-vel_rot);
			}

			if(GetAsyncKeyState(VK_DOWN))
			{
				escena.viewDir.rotar(cero,N,vel_rot);
				escena.V.rotar(cero,N,vel_rot);
				escena.U.rotar(cero,N,vel_rot);
			}
		}
		else
		{
			if(GetAsyncKeyState(VK_UP))
			{
				escena.viewDir.rotar(cero,BN,-vel_rot);
				escena.V.rotar(cero,BN,-vel_rot);
				escena.U.rotar(cero,BN,-vel_rot);
			}
			if(GetAsyncKeyState(VK_DOWN))
			{
				escena.viewDir.rotar(cero,BN,vel_rot);
				escena.V.rotar(cero,BN,vel_rot);
				escena.U.rotar(cero,BN,vel_rot);
			}
		}

		if(GetAsyncKeyState(VK_ADD))
			if(GetAsyncKeyState(VK_SHIFT))
				escena.voxel_step0*=1.01f;
			else
				escena.voxel_step*=1.01f;

		if(GetAsyncKeyState(VK_SUBTRACT))
			if(GetAsyncKeyState(VK_SHIFT))
				escena.voxel_step0/=1.01f;
			else
				escena.voxel_step/=1.01f;

		if(GetAsyncKeyState('W'))
			escena.lookFrom = escena.lookFrom + escena.viewDir*(elapsed_time*escena.vel_tras);
		if(GetAsyncKeyState('Z'))
			escena.lookFrom = escena.lookFrom - escena.viewDir*(elapsed_time*escena.vel_tras);

		escena.lookFrom.x = clamp256(escena.lookFrom.x);
		escena.lookFrom.y = clamp256(escena.lookFrom.y);
		escena.lookFrom.z = clamp256(escena.lookFrom.z);
		*/

		escena.Render();
		++cant_frames;

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
					break;
				case VK_PRIOR:
					break;
				case VK_ESCAPE:
					seguir = FALSE;
					break;
				}
				break;
			}

		}
	}

	exit(0);
}


void CChildView::OnFileOpen()
{
	CFileDialog objOpenFile(TRUE);
	if( IDOK != objOpenFile.DoModal())
	{
		exit(0);
	}

	if( !escena.tex.CreateFromFile( objOpenFile.GetPathName(), 256, 256,256))
	{
		AfxMessageBox( _T( "Failed to read the data" ));
	}


}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// almaceno la posicion inical del mouse
	x0 = point.x;
	y0 = point.y;
	ev_rotar = true;
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	ev_rotar = false;
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	if( ev_rotar )
	{
		x0 = point.x;
		y0 = point.y;
	}
	
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return TRUE;
}
