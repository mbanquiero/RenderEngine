#include "stdafx.h"
#include "/dev/graphics/Camera.h"

CBaseCamera::CBaseCamera()
{
	LA = LF = Vector3(0,0,0);
}


void CBaseCamera::Update()
{
	D3DXVECTOR3 up = D3DXVECTOR3(0,1,0);
	D3DXVECTOR3 position = (D3DXVECTOR3)LF;
	D3DXVECTOR3 lookAt = (D3DXVECTOR3)LA;
	D3DXMatrixLookAtLH(&m_viewMatrix, &position, &lookAt, &up);
}


CRotCamera::CRotCamera() : CBaseCamera()
{
	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}



void CRotCamera::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
}

D3DXVECTOR3 CRotCamera::GetRotation()
{
	return D3DXVECTOR3(m_rotationX, m_rotationY, m_rotationZ);
}


void CRotCamera::Update()
{
	LF.rotar(LA,m_rotationX, m_rotationY, m_rotationZ);
	CBaseCamera::Update();
}
