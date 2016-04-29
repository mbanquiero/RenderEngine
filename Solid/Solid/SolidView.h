
// SolidView.h: interfaz de la clase CSolidView
//

#pragma once

// Eventos
#define EV_NADA					0
#define EV_3D_REALTIME			1
#define EV_PAN_REALTIME			2

class CSolidView : public CView
{
protected: // Crear sólo a partir de serialización
	CSolidView();
	DECLARE_DYNCREATE(CSolidView)
// Atributos
public:
	CSolidDoc* GetDocument() const;
	bool primera_vez;
	int x0,y0;
	CPoint pos_cursor;
	int eventoInterno;

// Operaciones
public:

// Reemplazos
public:
	virtual void OnDraw(CDC* pDC);  // Reemplazado para dibujar esta vista
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	void RenderLoop();
	void ArmarEscena();
	void scaleToFit();


// Implementación
public:
	virtual ~CSolidView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Funciones de asignación de mensajes generadas
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	virtual void OnInitialUpdate();
	afx_msg void OnCargarTgcmesh();
	afx_msg void OnCargarTgcScene();
	afx_msg void OnPgdn();
	afx_msg void OnPgup();
};

#ifndef _DEBUG  // Versión de depuración en SolidView.cpp
inline CSolidDoc* CSolidView::GetDocument() const
   { return reinterpret_cast<CSolidDoc*>(m_pDocument); }
#endif

