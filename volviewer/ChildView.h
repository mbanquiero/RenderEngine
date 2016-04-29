
// ChildView.h: interfaz de la clase CChildView
//


#pragma once


// Ventana de CChildView

class CChildView : public CWnd
{
// Construcción
public:
	CChildView();

// Atributos
public:
	bool primera_vez;
	int x0,y0;		// pos. del mouse
	bool ev_rotar;
// Operaciones
public:

// Reemplazos
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementación
public:
	virtual ~CChildView();
	void RenderLoop();

	// Funciones de asignación de mensajes generadas
protected:
	afx_msg void OnPaint();
	afx_msg void OnFileOpen();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};

