
// Solid.h: archivo de encabezado principal para la aplicación Solid
//
#pragma once

#ifndef __AFXWIN_H__
	#error "incluir 'stdafx.h' antes de incluir este archivo para PCH"
#endif

#include "resource.h"       // Símbolos principales


// CSolidApp:
// Consulte la sección Solid.cpp para obtener información sobre la implementación de esta clase
//

class CSolidApp : public CWinAppEx
{
public:
	CSolidApp();


// Reemplazos
public:
	virtual BOOL InitInstance();

// Implementación
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CSolidApp theApp;
