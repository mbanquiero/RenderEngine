
// rayt.h: archivo de encabezado principal para la aplicación PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "incluir 'stdafx.h' antes de incluir este archivo para PCH"
#endif

#include "resource.h"		// Símbolos principales


// CraytApp:
// Consulte la sección rayt.cpp para obtener información sobre la implementación de esta clase
//

class CraytApp : public CWinApp
{
public:
	CraytApp();

// Reemplazos
public:
	virtual BOOL InitInstance();

// Implementación

	DECLARE_MESSAGE_MAP()
};

extern CraytApp theApp;