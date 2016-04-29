
// volviewer.h: archivo de encabezado principal para la aplicación volviewer
//
#pragma once

#ifndef __AFXWIN_H__
	#error "incluir 'stdafx.h' antes de incluir este archivo para PCH"
#endif

#include "resource.h"       // Símbolos principales


// CvolviewerApp:
// Consulte la sección volviewer.cpp para obtener información sobre la implementación de esta clase
//

class CvolviewerApp : public CWinApp
{
public:
	CvolviewerApp();


// Reemplazos
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementación

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CvolviewerApp theApp;
