
// rayt.h: archivo de encabezado principal para la aplicaci�n PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "incluir 'stdafx.h' antes de incluir este archivo para PCH"
#endif

#include "resource.h"		// S�mbolos principales


// CraytApp:
// Consulte la secci�n rayt.cpp para obtener informaci�n sobre la implementaci�n de esta clase
//

class CraytApp : public CWinApp
{
public:
	CraytApp();

// Reemplazos
public:
	virtual BOOL InitInstance();

// Implementaci�n

	DECLARE_MESSAGE_MAP()
};

extern CraytApp theApp;