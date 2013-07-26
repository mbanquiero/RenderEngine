#pragma once
#include <d3d11.h>
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\dxgi.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dcommon.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx11.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10Math.h"

class CRenderEngine;

class CGraphicObject
{
public:
	char obj_name[32];		// nombre del objeto
	CRenderEngine *M;		// puntero al modelo
	int tipo;
	int id;					// id del objeto, para el combined id (puede no ser unico, para ciertos efectos de sombras) = nro_objeto
	int serial_id;			// id unico (nro correlativo)
	unsigned long idata;	// item data (usualmente un puntero)

	D3DXVECTOR3 m_pos;		// Origen
	D3DXVECTOR3 m_size;		// Tamaño
	D3DXVECTOR3 m_rot;		// Orientacion
	D3DXMATRIX matWorld;		

	int nro_mesh;				// mesh representa el subindice dentro del vector model::Mesh[]

	CGraphicObject();
	virtual void CalcularMatriz(bool transponer);
	virtual void Render();

	virtual void Scale(float escala);

};


