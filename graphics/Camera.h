#pragma once

#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx10Math.h"
#include "/dev/graphics/vectores.h"


class CBaseCamera
{
public:
	Vector3 LF,LA;
	D3DXMATRIX m_viewMatrix;
	CBaseCamera();
	virtual void Update();					// Computa la matrix de view

};


class CRotCamera : public CBaseCamera
{
public:
	float m_rotationX, m_rotationY, m_rotationZ;
	CRotCamera();
	virtual D3DXVECTOR3 GetRotation();
	virtual void SetRotation(float, float, float);
	virtual void Update();					// Computa la matrix de view

};