
#include <GL\glew.h>
#include <GL\GL.h>

#pragma once


class CRenderer
{
public:
	CRenderer(void);
	virtual ~CRenderer(void);

	bool Initialize( HDC hContext_i);
	void Resize( int nWidth_i, int nHeight_i );
	void Render();

	HGLRC m_hglContext;
};

