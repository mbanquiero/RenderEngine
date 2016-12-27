
// rayt.cpp : define los comportamientos de las clases para la aplicación.
//

#include "stdafx.h"
#include "rayt.h"
#include "raytDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CraytApp

BEGIN_MESSAGE_MAP(CraytApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// Construcción de CraytApp

CraytApp::CraytApp()
{
	// TODO: agregar aquí el código de construcción,
	// Colocar toda la inicialización importante en InitInstance
}


// El único objeto CraytApp

CraytApp theApp;


// Inicialización de CraytApp

BOOL CraytApp::InitInstance()
{
	CWinApp::InitInstance();


	// Crear el administrador de shell, en caso de que el cuadro de diálogo contenga
	// alguna vista de árbol de shell o controles de vista de lista de shell.
	CShellManager *pShellManager = new CShellManager;

	// Activar el administrador visual "nativo de Windows" para habilitar temas en controles MFC
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Inicialización estándar
	// Si no utiliza estas características y desea reducir el tamaño
	// del archivo ejecutable final, debe quitar
	// las rutinas de inicialización específicas que no necesite
	// Cambie la clave del Registro en la que se almacena la configuración
	// TODO: debe modificar esta cadena para que contenga información correcta
	// como el nombre de su compañía u organización
	SetRegistryKey(_T("Aplicaciones generadas con el Asistente para aplicaciones local"));

	CraytDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: insertar aquí el código para controlar
		//  cuándo se descarta el cuadro de diálogo con Aceptar
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: insertar aquí el código para controlar
		//  cuándo se descarta el cuadro de diálogo con Cancelar
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Advertencia: la aplicación está finalizando porque hubo un error al crear el cuadro de diálogo.\n");
		TRACE(traceAppMsg, 0, "Advertencia: si usa controles MFC en el cuadro de diálogo, no puede usar #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Eliminar el administrador de shell creado anteriormente.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Dado que el cuadro de diálogo se ha cerrado, devolver FALSE para salir
	//  de la aplicación en vez de iniciar el suministro de mensajes de dicha aplicación.
	return FALSE;
}

