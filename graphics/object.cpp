#include "stdafx.h"
#include "/dev/graphics/object.h"
#include "/dev/graphics/renderEngine.h"


CGraphicObject::CGraphicObject()
{
	M = NULL;
	strcpy(obj_name,"");
	tipo = 0;
	id = 0;
	serial_id = 0;
	idata = -1;
	m_pos = D3DXVECTOR3(0,0,0);
	m_size = D3DXVECTOR3(0,0,0);
	m_rot = D3DXVECTOR3(0,0,0);
	nro_mesh = -1;
}


void CGraphicObject::CalcularMatriz(bool transponer)
{
	D3DXMatrixIdentity(&matWorld);

	// Acceso al mesh que representa el objeto
	CBaseMesh *p_mesh = M->m_mesh[nro_mesh];
	// determino la escala del objeto, (para que el mesh ocupe exactamente el tamaño del objeto)
	// si tiene cero el size del objeto, usa el del mesh
	float sx = p_mesh->m_size.x;
	float sy = p_mesh->m_size.y;
	float sz = p_mesh->m_size.z;
	float kx = sx && m_size.x?m_size.x/sx:1;
	float ky = sy && m_size.y?m_size.y/sy:1;
	float kz = sz && m_size.z?m_size.z/sz:1;

	// 1- lo llevo al cero en el espacio del objeto pp dicho
	// La traslacion T0 hace que el centro del objeto quede en el cero
	// size = tamaño y P0 punto inicial, en coordenadas del objeto. 
	D3DXMATRIXA16 T0;
	D3DXMatrixTranslation(&T0,-p_mesh->m_pos.x-sx/2,-p_mesh->m_pos.y-sy/2,-p_mesh->m_pos.z-sz/2);
	D3DXMatrixMultiply(&matWorld,&matWorld,&T0);

	// 2- lo escalo
	D3DXMATRIXA16 Es;
	D3DXMatrixScaling(&Es,kx,ky,kz);
	D3DXMatrixMultiply(&matWorld,&matWorld,&Es);

	// 3- lo roto
	D3DXMATRIXA16 Rx,Ry,Rz;
	D3DXMatrixRotationZ(&Rz,m_rot.z);
	D3DXMatrixRotationY(&Ry,m_rot.y);
	D3DXMatrixRotationX(&Rx,m_rot.x);
	D3DXMatrixMultiply(&matWorld,&matWorld,&Rx);
	D3DXMatrixMultiply(&matWorld,&matWorld,&Ry);
	D3DXMatrixMultiply(&matWorld,&matWorld,&Rz);

	// 4- Lo traslado a la posicion final
	D3DXMATRIXA16 T;
	D3DXMatrixTranslation(&T,m_pos.x,m_pos.y,m_pos.z);
	D3DXMatrixMultiply(&matWorld,&matWorld,&T);

	if(transponer)
		// ojo, las matrices en los shader de directx 11 van transpuestas!! 
		D3DXMatrixTranspose(&matWorld, &matWorld);

}


void CGraphicObject::Render()
{
	// Seteo las matrices en el engine
	M->SetTransformWorld(matWorld);
	// Y actualizo los parametros del shader
	M->SetMatrixBuffer();
	// Ahora si dibujo la malla pp dicha
	// Acceso al mesh que representa el objeto
	CBaseMesh *p_mesh = M->m_mesh[nro_mesh];
	p_mesh->Draw();
}


void CGraphicObject::Scale(float escala)
{
	m_size = m_size * escala;
	// Aprovecho para calcular la matriz de world de este objeto
	CalcularMatriz(M->device->transponer_matrices);
}
