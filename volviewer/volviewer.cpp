
// volviewer.cpp : define los comportamientos de las clases para la aplicación.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "volviewer.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CvolviewerApp

BEGIN_MESSAGE_MAP(CvolviewerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CvolviewerApp::OnAppAbout)
END_MESSAGE_MAP()


// Construcción de CvolviewerApp

CvolviewerApp::CvolviewerApp()
{
	// TODO: reemplace la cadena de identificador de aplicación siguiente por una cadena de identificador único; el formato
	// recomendado para la cadena es NombreCompañía.NombreProducto.Subproducto.InformaciónDeVersión
	SetAppID(_T("volviewer.AppID.NoVersion"));

	// TODO: agregar aquí el código de construcción,
	// Colocar toda la inicialización importante en InitInstance
}

// El único objeto CvolviewerApp

CvolviewerApp theApp;


// Inicialización de CvolviewerApp

BOOL CvolviewerApp::InitInstance()
{
	CWinApp::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// Se necesita AfxInitRichEdit2() para usar el control RichEdit	
	// AfxInitRichEdit2();

	// Inicialización estándar
	// Si no utiliza estas características y desea reducir el tamaño
	// del archivo ejecutable final, debe quitar
	// las rutinas de inicialización específicas que no necesite
	// Cambie la clave del Registro en la que se almacena la configuración
	// TODO: debe modificar esta cadena para que contenga información correcta
	// como el nombre de su compañía u organización
	SetRegistryKey(_T("Aplicaciones generadas con el Asistente para aplicaciones local"));


	// Para crear la ventana principal, este código crea un nuevo objeto de ventana
	// de marco y, a continuación, lo establece como el objeto de ventana principal de la aplicación
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// Crear y cargar el marco con sus recursos
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);





	// Se ha inicializado la única ventana; mostrarla y actualizarla
	pFrame->ShowWindow(SW_SHOWMAXIMIZED);
	pFrame->UpdateWindow();
	return TRUE;
}

int CvolviewerApp::ExitInstance()
{
	//TODO: controlar recursos adicionales que se hayan podido agregar
	return CWinApp::ExitInstance();
}

// Controladores de mensaje de CvolviewerApp


// Cuadro de diálogo CAboutDlg utilizado para el comando Acerca de

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Datos del cuadro de diálogo
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX/DDV

// Implementación
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// Comando de la aplicación para ejecutar el cuadro de diálogo
void CvolviewerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// Controladores de mensaje de CvolviewerApp



