
// raytDlg.h: archivo de encabezado
//

#pragma once


// Cuadro de diálogo de CraytDlg
class CraytDlg : public CDialogEx
{
// Construcción
public:
	CraytDlg(CWnd* pParent = NULL);	// Constructor estándar

// Datos del cuadro de diálogo
	enum { IDD = IDD_RAYT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// Compatibilidad con DDX/DDV


// Implementación
protected:
	HICON m_hIcon;

	// Funciones de asignación de mensajes generadas
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedTestSse();
};
