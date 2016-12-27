
// raytDlg.cpp: archivo de implementación
//

#include "stdafx.h"
#include "rayt.h"
#include "raytDlg.h"
#include "afxdialogex.h"
#include "engine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CEngine motor;
// Cuadro de diálogo de CraytDlg



CraytDlg::CraytDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CraytDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CraytDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CraytDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CraytDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_TEST_SSE, &CraytDlg::OnBnClickedTestSse)
END_MESSAGE_MAP()


// Controladores de mensaje de CraytDlg

BOOL CraytDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	motor.initFromTest();

	// Establecer el icono para este cuadro de diálogo. El marco de trabajo realiza esta operación
	//  automáticamente cuando la ventana principal de la aplicación no es un cuadro de diálogo
	SetIcon(m_hIcon, TRUE);			// Establecer icono grande
	SetIcon(m_hIcon, FALSE);		// Establecer icono pequeño


	return TRUE;  // Devuelve TRUE  a menos que establezca el foco en un control
}

// Si agrega un botón Minimizar al cuadro de diálogo, necesitará el siguiente código
//  para dibujar el icono. Para aplicaciones MFC que utilicen el modelo de documentos y vistas,
//  esta operación la realiza automáticamente el marco de trabajo.

void CraytDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Contexto de dispositivo para dibujo

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Centrar icono en el rectángulo de cliente
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Dibujar el icono
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// El sistema llama a esta función para obtener el cursor que se muestra mientras el usuario arrastra
//  la ventana minimizada.
HCURSOR CraytDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CraytDlg::OnBnClickedOk()
{
	CDC *pDC = GetDC();

	LARGE_INTEGER F,T0,T1;   // address of current frequency
	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);
	//motor.renderOctree(pDC);
	//motor.render(pDC);
	motor.renderKDTree(pDC);

	QueryPerformanceCounter(&T1);
	float elapsed_time = (float)(T1.QuadPart - T0.QuadPart) / (float)F.QuadPart;
	char buffer[255];
	sprintf(buffer,"%g s" , elapsed_time);
	AfxMessageBox(buffer);
	ReleaseDC(pDC);
}


void CraytDlg::OnBnClickedTestSse()
{
	vec3 A = vec3(1,1,1);
	vec3 B = vec3(2,-1,-2);

	__m128 C = _mm_setr_ps(1,1,1,0);
	__m128 D = _mm_setr_ps(2,-1,-2,0);

	LARGE_INTEGER F,T0,T1;   // address of current frequency
	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);
	for(int i=0;i<10000000;++i)
		CrossProduct( C , D);
	QueryPerformanceCounter(&T1);
	float elapsed_timeA = (float)(T1.QuadPart - T0.QuadPart) / (float)F.QuadPart;
	T0 = T1;
	for(int i=0;i<10000000;++i)
		cross( A , B);
	QueryPerformanceCounter(&T1);
	float elapsed_timeB = (float)(T1.QuadPart - T0.QuadPart) / (float)F.QuadPart;

	char buffer[255];
	sprintf(buffer,"Fast= %g s       Normal=%g s" , elapsed_timeA,elapsed_timeB);
	AfxMessageBox(buffer);


}
